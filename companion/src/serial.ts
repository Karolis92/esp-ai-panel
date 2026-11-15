// Serial link to the panel: auto-detect, auto-reconnect, NDJSON framing.
import { EventEmitter } from "node:events";
import { SerialPort, ReadlineParser } from "serialport";
import { DeviceMsg, HostMsg, encode } from "./protocol.js";

/** USB vendor ids: WCH CH340 (UART bridge on the 4848S040) and Espressif native USB. */
const KNOWN_VIDS = ["1a86", "303a", "10c4"];
export const DEFAULT_BAUD = 115200;

export async function findPort(): Promise<string | null> {
  const ports = await SerialPort.list();
  const hit = ports.find((p) => p.vendorId && KNOWN_VIDS.includes(p.vendorId.toLowerCase()));
  if (!hit) return null;
  // On macOS prefer the callout device (/dev/cu.*) over the dial-in one (/dev/tty.*).
  return hit.path.replace("/dev/tty.", "/dev/cu.");
}

export interface SerialLinkEvents {
  connected: [path: string];
  disconnected: [];
  message: [msg: DeviceMsg];
  log: [line: string];
}

export class SerialLink extends EventEmitter<SerialLinkEvents> {
  private port: SerialPort | null = null;
  private timer: NodeJS.Timeout | null = null;
  private stopped = false;
  path: string | null = null;
  fw: string | null = null;

  constructor(
    private opts: { port?: string; baud?: number; retryMs?: number } = {},
  ) {
    super();
  }

  get isOpen(): boolean {
    return !!this.port?.isOpen;
  }

  start(): void {
    this.stopped = false;
    void this.tryOpen();
  }

  stop(): void {
    this.stopped = true;
    if (this.timer) clearTimeout(this.timer);
    this.port?.close(() => {});
    this.port = null;
  }

  write(msg: HostMsg): void {
    if (!this.port?.isOpen) return;
    this.port.write(encode(msg));
  }

  private schedule(): void {
    if (this.stopped) return;
    if (this.timer) clearTimeout(this.timer);
    this.timer = setTimeout(() => void this.tryOpen(), this.opts.retryMs ?? 2000);
  }

  private async tryOpen(): Promise<void> {
    if (this.stopped || this.port) return;
    let path: string | null = this.opts.port ?? null;
    if (!path) {
      try {
        path = await findPort();
      } catch {
        path = null;
      }
    }
    if (!path) return this.schedule();

    const port = new SerialPort({ path, baudRate: this.opts.baud ?? DEFAULT_BAUD, autoOpen: false, hupcl: false });
    port.open((err) => {
      if (err) {
        this.emit("log", `open ${path} failed: ${err.message}`);
        return this.schedule();
      }
      // Opening asserts DTR/RTS, which the board's auto-reset circuit turns into
      // EN/IO0 (reset / download mode). Release both, like idf.py monitor does.
      port.set({ dtr: false, rts: false }, (e) => e && this.emit("log", `set dtr/rts: ${e.message}`));
      this.port = port;
      this.path = path;
      this.emit("connected", path!);
      const parser = port.pipe(new ReadlineParser({ delimiter: "\n" }));
      parser.on("data", (line: string) => this.onLine(line));
    });
    port.on("close", () => {
      if (this.port === port) {
        this.port = null;
        this.fw = null;
        this.emit("disconnected");
        this.schedule();
      }
    });
    port.on("error", (e) => this.emit("log", `serial error: ${e.message}`));
  }

  private onLine(raw: string): void {
    const line = raw.trim();
    // Boot ROM / ESP log output also arrives on this port — only JSON objects are protocol.
    if (!line.startsWith("{")) {
      if (line) this.emit("log", `dev: ${line}`);
      return;
    }
    try {
      const msg = JSON.parse(line) as DeviceMsg;
      if (msg.t === "hello") this.fw = msg.fw;
      this.emit("message", msg);
    } catch {
      this.emit("log", `dev (bad json): ${line}`);
    }
  }
}
