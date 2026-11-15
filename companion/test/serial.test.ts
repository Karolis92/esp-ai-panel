import { afterEach, beforeEach, describe, expect, it, vi } from "vitest";
import { PortHandle, PROBE_TIMEOUT_MS, REJECT_COOLDOWN_MS, SerialLink } from "../src/serial.js";

/** Fake serial device. `panel: true` answers every ping with hello, like the firmware. */
class FakePort implements PortHandle {
  written: string[] = [];
  open = true;
  private lineCbs: ((l: string) => void)[] = [];
  private closeCbs: (() => void)[] = [];
  constructor(public path: string, private panel: boolean) {}
  write(d: string) {
    this.written.push(d);
    if (this.panel && d.includes('"t":"ping"')) queueMicrotask(() => this.emitLine('{"t":"hello","fw":"0.1.0"}'));
  }
  close() {
    if (!this.open) return;
    this.open = false;
    this.closeCbs.forEach((cb) => cb());
  }
  onLine(cb: (l: string) => void) {
    this.lineCbs.push(cb);
  }
  onClose(cb: () => void) {
    this.closeCbs.push(cb);
  }
  onError() {}
  emitLine(l: string) {
    this.lineCbs.forEach((cb) => cb(l));
  }
  /** Everything except probe pings. */
  get data() {
    return this.written.filter((d) => !d.includes('"t":"ping"'));
  }
}

function setup(devices: Record<string, boolean>, opts: { port?: string } = {}) {
  const opened: FakePort[] = [];
  let present = Object.keys(devices);
  const link = new SerialLink({
    ...opts,
    retryMs: 2000,
    now: () => Date.now(),
    list: async () => present,
    open: async (path) => {
      if (!(path in devices)) throw new Error("No such file or directory");
      const p = new FakePort(path, devices[path]);
      opened.push(p);
      return p;
    },
  });
  const events: string[] = [];
  link.on("connected", (p) => events.push(`connected ${p}`));
  link.on("disconnected", () => events.push("disconnected"));
  const state = { t: "state", now: 0, sessions: [], usage: {} } as never;
  return { link, opened, events, state, setPresent: (p: string[]) => (present = p) };
}

describe("SerialLink port probing", () => {
  beforeEach(() => vi.useFakeTimers());
  afterEach(() => vi.useRealTimers());

  it("connects to a port that answers hello", async () => {
    const { link, events, opened, state } = setup({ "/dev/panel": true });
    link.start();
    await vi.advanceTimersByTimeAsync(10);
    expect(events).toEqual(["connected /dev/panel"]);
    expect(link.isOpen).toBe(true);
    expect(link.fw).toBe("0.1.0");
    link.write(state);
    expect(opened[0].data).toHaveLength(1);
    link.stop();
  });

  it("skips a device that never answers and sends it nothing but pings", async () => {
    const { link, events, opened, state } = setup({ "/dev/arduino": false, "/dev/panel": true });
    link.start();
    link.write(state); // not connected yet: dropped
    await vi.advanceTimersByTimeAsync(PROBE_TIMEOUT_MS + 10);
    expect(events).toEqual(["connected /dev/panel"]);
    const arduino = opened.find((p) => p.path === "/dev/arduino")!;
    expect(arduino.open).toBe(false);
    expect(arduino.written.length).toBeGreaterThan(0);
    expect(arduino.data).toEqual([]);
    link.stop();
  });

  it("puts a silent device on cooldown instead of reopening it every retry", async () => {
    const { link, opened, setPresent } = setup({ "/dev/arduino": false, "/dev/panel": true });
    setPresent(["/dev/arduino"]);
    link.start();
    await vi.advanceTimersByTimeAsync(PROBE_TIMEOUT_MS + 10);
    await vi.advanceTimersByTimeAsync(REJECT_COOLDOWN_MS / 2);
    expect(opened.filter((p) => p.path === "/dev/arduino")).toHaveLength(1);
    expect(link.isOpen).toBe(false);

    // Panel plugged in later: picked up on the next retry.
    setPresent(["/dev/arduino", "/dev/panel"]);
    await vi.advanceTimersByTimeAsync(2010);
    expect(link.path).toBe("/dev/panel");

    link.stop();
  });

  it("probes the arduino again after the cooldown", async () => {
    const { link, opened, setPresent } = setup({ "/dev/arduino": false });
    setPresent(["/dev/arduino"]);
    link.start();
    await vi.advanceTimersByTimeAsync(PROBE_TIMEOUT_MS + REJECT_COOLDOWN_MS + 2500);
    expect(opened.length).toBe(2);
    link.stop();
  });

  it("a configured port must also answer, but is retried without cooldown", async () => {
    const { link, opened, events } = setup({ "/dev/fixed": false }, { port: "/dev/fixed" });
    link.start();
    await vi.advanceTimersByTimeAsync(PROBE_TIMEOUT_MS + 2010);
    expect(events).toEqual([]);
    expect(opened.length).toBe(2);
    link.stop();
  });

  it("reconnects after the panel disconnects", async () => {
    const { link, opened, events } = setup({ "/dev/panel": true });
    link.start();
    await vi.advanceTimersByTimeAsync(10);
    opened[0].close();
    await vi.advanceTimersByTimeAsync(2010);
    expect(events).toEqual(["connected /dev/panel", "disconnected", "connected /dev/panel"]);
    link.stop();
  });

  it("emits device messages after connecting", async () => {
    const { link, opened } = setup({ "/dev/panel": true });
    const msgs: unknown[] = [];
    link.on("message", (m) => msgs.push(m));
    link.start();
    await vi.advanceTimersByTimeAsync(10);
    opened[0].emitLine('{"t":"tap","id":"abc"}');
    opened[0].emitLine("I (123) boot: log line");
    expect(msgs).toEqual([{ t: "tap", id: "abc" }]);
    link.stop();
  });
});
