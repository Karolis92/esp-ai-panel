import { startServer, IpcRequest } from "./ipc.js";
import { readConfig } from "./paths.js";
import { SerialLink } from "./serial.js";
import { HookEvent, SessionStore, StatusLineInput } from "./sessions.js";

const DEBOUNCE_MS = 100;
const PING_MS = 5000;
const TICK_MS = 250; // scene hold / long-command timers

function log(msg: string): void {
  console.log(`${new Date().toISOString()} ${msg}`);
}

export function runDaemon(opts: { verbose?: boolean } = {}): void {
  const cfg = readConfig();
  const store = new SessionStore();
  const link = new SerialLink({ port: cfg.port, baud: cfg.baud });

  let pending: NodeJS.Timeout | null = null;
  const push = () => {
    if (pending) return;
    pending = setTimeout(() => {
      pending = null;
      const snap = store.snapshot();
      link.write(snap);
      if (opts.verbose) log(`→ ${JSON.stringify(snap)}`);
    }, DEBOUNCE_MS);
  };

  link.on("connected", (p) => {
    log(`panel connected on ${p}`);
    push();
  });
  link.on("disconnected", () => log("panel disconnected"));
  link.on("log", (l) => opts.verbose && log(l));
  link.on("message", (m) => {
    if (m.t === "hello") {
      if (opts.verbose) log(`← hello fw ${m.fw}`);
      push();
    } else if (m.t === "tap") {
      log(`← tap ${m.id}`);
    }
  });

  const handle = (req: IpcRequest): unknown => {
    switch (req.kind) {
      case "hook": {
        const ev = req.payload as HookEvent;
        if (opts.verbose) log(`hook ${ev?.hook_event_name} ${ev?.session_id?.slice(0, 8)} ${ev?.tool_name ?? ""}`);
        if (store.applyHook(ev)) push();
        return undefined;
      }
      case "statusline":
        if (store.applyStatusLine(req.payload as StatusLineInput)) push();
        return undefined;
      case "status":
        return {
          panel: link.isOpen ? { port: link.path, fw: link.fw } : null,
          state: store.snapshot(),
        };
    }
  };

  const server = startServer(handle);
  server.on("error", (e) => {
    log(`ipc server error: ${e.message}`);
    process.exit(1);
  });
  link.start();

  setInterval(() => {
    if (store.tick()) push();
  }, TICK_MS).unref();
  setInterval(() => link.write({ t: "ping", now: Math.floor(Date.now() / 1000) }), PING_MS);

  const shutdown = () => {
    log("shutting down");
    link.stop();
    server.close();
    process.exit(0);
  };
  process.on("SIGINT", shutdown);
  process.on("SIGTERM", shutdown);
  log("ai-panel daemon started");
}
