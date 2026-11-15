// `ai-panel demo` — drives the panel with fake data reproducing design.png (daemon must be stopped).
import { StateMsg, WireSession } from "./protocol.js";
import { readConfig } from "./paths.js";
import { SerialLink } from "./serial.js";

export function demoState(step: number): StateMsg {
  const now = Math.floor(Date.now() / 1000);
  const page1: WireSession[] = [
    { id: "d1", title: "API-SERVER", activity: "Editing auth.ts", state: "working", anim: "laptop" },
    { id: "d2", title: "WEB-APP", activity: "Needs you", state: "needs", anim: "alert" },
    { id: "d3", title: "CLI-TOOLS", activity: "Running npm test", state: "working", anim: "terminal" },
    { id: "d4", title: "MOBILE-APP", activity: "Planning…", state: "working", anim: "plan" },
    { id: "d5", title: "BACKEND", activity: "Researching…", state: "working", anim: "book" },
    { id: "d6", title: "BLOG", activity: "Done", state: "done", anim: "done" },
  ];
  const extra: WireSession[] = [
    { id: "d7", title: "ESP-AI-PANEL", activity: "Done", state: "done", anim: "done" },
    { id: "d8", title: "BILLING-API", activity: "Error", state: "error", anim: "error" },
    { id: "d9", title: "DOCS", activity: "Thinking…", state: "working", anim: "thinking" },
    { id: "d10", title: "INFRA", activity: "Running terraform apply", state: "working", anim: "coffee" },
    { id: "d11", title: "SCRIPTS", activity: "Waiting…", state: "idle", anim: "sleep" },
    { id: "d12", title: "DATA-PIPE", activity: "Delegating…", state: "working", anim: "thinking" },
    { id: "d13", title: "WEBSITE", activity: "Waiting…", state: "idle", anim: "sleep" },
  ];
  // Every 10 s flip the "needs" card back to working to exercise transitions.
  const flip = Math.floor(step / 5) % 2 === 1;
  if (flip) page1[1] = { ...page1[1], state: "working", anim: "thinking", activity: "Thinking…" };
  return {
    t: "state",
    now,
    sessions: [...page1, ...extra],
    usage: {
      s5h: { pct: 45, reset: now + 2 * 3600 + 15 * 60 },
      wk: { pct: 28, reset: now + 5 * 86400 + 8 * 3600 },
    },
  };
}

export function runDemo(): void {
  const cfg = readConfig();
  const link = new SerialLink({ port: cfg.port, baud: cfg.baud });
  let step = 0;
  link.on("connected", (p) => {
    console.log(`demo: connected to ${p}`);
    link.write(demoState(step));
  });
  link.on("disconnected", () => console.log("demo: disconnected"));
  link.on("log", (l) => console.log(l));
  link.on("message", (m) => {
    console.log(`demo: ← ${JSON.stringify(m)}`);
    if (m.t === "hello") link.write(demoState(step));
  });
  link.start();
  setInterval(() => {
    step++;
    link.write(demoState(step));
  }, 2000);
  console.log("demo: waiting for panel… (Ctrl+C to quit; stop the daemon first)");
}
