// Serial link to the panel: auto-detect, handshake, auto-reconnect, NDJSON framing.
//
// A port only counts as the panel after it answers our `ping` with `{"t":"hello"}`. Until then
// nothing but pings is written to it, so an unrelated USB-serial device (Arduino, printer, …)
// never receives session data and is released again after PROBE_TIMEOUT_MS.
import { EventEmitter } from "node:events";
import { SerialPort, ReadlineParser } from "serialport";
import { DeviceMsg, HostMsg, encode } from "./protocol.js";

/** USB vendor ids: WCH CH340 (UART bridge on the 4848S040), Espressif native USB, SiLabs CP210x. */
const KNOWN_VIDS = ["1a86", "303a", "10c4"];
export const DEFAULT_BAUD = 115200;
/** How long a candidate port gets to answer with `hello`. Covers a panel that is still booting. */
export const PROBE_TIMEOUT_MS = 4000;
/** Interval between pings while probing. */
export const PROBE_PING_MS = 1000;
/** An auto-detected port that didn't answer isn't probed again for this long. */
export const REJECT_COOLDOWN_MS = 60_000;

/** Minimal port surface, so tests can substitute a fake. */
export interface PortHandle {
  write(data: string): void;
  close(): void;
  onLine(cb: (line: string) => void): void;
  onClose(cb: () => void): void;
  onError(cb: (err: Error) => void): void;
}
export type OpenPort = (path: string, baud: number) => Promise<PortHandle>;
export type ListPorts = () => Promise<string[]>;

/** All serial ports with a known vendor id, in OS listing order. */
export async function listCandidates(): Promise<string[]> {
  const ports = await SerialPort.list();
  return (
    ports
      .filter((p) => p.vendorId && KNOWN_VIDS.includes(p.vendorId.toLowerCase()))
      // On macOS prefer the callout device (/dev/cu.*) over the dial-in one (/dev/tty.*).
      .map((p) => p.path.replace("/dev/tty.", "/dev/cu."))
  );
}

export const openSerialPort: OpenPort = (path, baud) =>
  new Promise((resolve, reject) => {
    const port = new SerialPort({ path, baudRate: baud, autoOpen: false, hupcl: false });
    port.open((err) => {
      if (err) return reject(err);
      // Opening asserts DTR/RTS, which the board's auto-reset circuit turns into
      // EN/IO0 (reset / download mode). Release both, like idf.py monitor does.
      port.set({ dtr: false, rts: false }, () => {});
      const parser = port.pipe(new ReadlineParser({ delimiter: "\n" }));
      resolve({
        write: (d) => void port.write(d),
        close: () => {
          if (port.isOpen) port.close(() => {});
        },
        onLine: (cb) => void parser.on("data", cb),
        onClose: (cb) => void port.on("close", cb),
        onError: (cb) => void port.on("error", cb),
      });
    });
  });

export interface SerialLinkEvents {
  connected: [path: string];
  disconnected: [];
  message: [msg: DeviceMsg];
  log: [line: string];
}

export interface SerialLinkOptions {
  /** Fixed port from config. It still has to answer `hello`, but never goes on cooldown. */
  port?: string;
  baud?: number;
  retryMs?: number;
  /** Injection points for tests. */
  list?: ListPorts;
  open?: OpenPort;
  now?: () => number;
}

export class SerialLink extends EventEmitter<SerialLinkEvents> {
  private port: PortHandle | null = null;
  private timer: NodeJS.Timeout | null = null;
  private stopped = true;
  private busy = false;
  private rejected = new Map<string, number>(); // path → don't probe again before this time
  path: string | null = null;
  fw: string | null = null;

  constructor(private opts: SerialLinkOptions = {}) {
    super();
  }

  /** True once a port has answered with `hello`. */
  get isOpen(): boolean {
    return !!this.port;
  }

  start(): void {
    this.stopped = false;
    void this.tryConnect();
  }

  stop(): void {
    this.stopped = true;
    if (this.timer) clearTimeout(this.timer);
    this.timer = null;
    const p = this.port;
    this.port = null;
    p?.close();
  }

  /** Writes only to a verified panel; dropped otherwise. */
  write(msg: HostMsg): void {
    this.port?.write(encode(msg));
  }

  private now(): number {
    return (this.opts.now ?? Date.now)();
  }

  private schedule(): void {
    if (this.stopped) return;
    if (this.timer) clearTimeout(this.timer);
    this.timer = setTimeout(() => {
      this.timer = null;
      void this.tryConnect();
    }, this.opts.retryMs ?? 2000);
  }

  private async tryConnect(): Promise<void> {
    if (this.stopped || this.port || this.busy) return;
    this.busy = true;
    try {
      let candidates: string[];
      if (this.opts.port) {
        candidates = [this.opts.port];
      } else {
        try {
          candidates = await (this.opts.list ?? listCandidates)();
        } catch {
          candidates = [];
        }
        const t = this.now();
        candidates = candidates.filter((p) => (this.rejected.get(p) ?? 0) <= t);
      }
      for (const path of candidates) {
        if (this.stopped) return;
        if (await this.probe(path)) return;
      }
    } finally {
      this.busy = false;
    }
    this.schedule();
  }

  /** Open `path`, ping it and wait for `hello`. On success the port becomes the live link. */
  private async probe(path: string): Promise<boolean> {
    let port: PortHandle;
    try {
      port = await (this.opts.open ?? openSerialPort)(path, this.opts.baud ?? DEFAULT_BAUD);
    } catch (e) {
      // Busy or gone. That says nothing about what the device is, so no cooldown.
      this.emit("log", `open ${path} failed: ${(e as Error).message}`);
      return false;
    }
    if (this.stopped) {
      port.close();
      return false;
    }
    this.emit("log", `probing ${path}`);

    let probing = true;
    const hello = await new Promise<string | null>((resolve) => {
      const finish = (fw: string | null) => {
        if (!probing) return;
        probing = false;
        clearInterval(pinger);
        clearTimeout(timeout);
        resolve(fw);
      };
      const ping = () => port.write(encode({ t: "ping", now: Math.floor(this.now() / 1000) }));
      const pinger = setInterval(ping, PROBE_PING_MS);
      const timeout = setTimeout(() => finish(null), PROBE_TIMEOUT_MS);
      port.onLine((raw) => {
        if (!probing) return this.onLine(raw);
        const msg = parseMsg(raw);
        if (msg?.t === "hello") finish(msg.fw);
      });
      port.onClose(() => finish(null));
      port.onError((e) => this.emit("log", `serial error on ${path}: ${e.message}`));
      ping();
    });

    if (hello === null || this.stopped) {
      port.close();
      if (hello === null) {
        this.emit("log", `${path}: no hello, not the panel; skipping`);
        if (!this.opts.port) this.rejected.set(path, this.now() + REJECT_COOLDOWN_MS);
      }
      return false;
    }

    this.port = port;
    this.path = path;
    this.fw = hello;
    this.rejected.clear();
    port.onClose(() => {
      if (this.port !== port) return;
      this.port = null;
      this.fw = null;
      this.emit("disconnected");
      this.schedule();
    });
    this.emit("connected", path);
    return true;
  }

  private onLine(raw: string): void {
    const line = raw.trim();
    // Boot ROM / ESP log output also arrives on this port — only JSON objects are protocol.
    if (!line.startsWith("{")) {
      if (line) this.emit("log", `dev: ${line}`);
      return;
    }
    const msg = parseMsg(line);
    if (!msg) {
      this.emit("log", `dev (bad json): ${line}`);
      return;
    }
    if (msg.t === "hello") this.fw = msg.fw;
    this.emit("message", msg);
  }
}

function parseMsg(raw: string): DeviceMsg | null {
  const line = raw.trim();
  if (!line.startsWith("{")) return null;
  try {
    const msg = JSON.parse(line) as DeviceMsg;
    return msg && typeof msg === "object" && typeof msg.t === "string" ? msg : null;
  } catch {
    return null;
  }
}
