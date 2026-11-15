#!/usr/bin/env node
import { spawn } from "node:child_process";
import { runDaemon } from "./daemon.js";
import { runDemo } from "./demo.js";
import { install, uninstall } from "./install.js";
import { send } from "./ipc.js";
import { readConfig } from "./paths.js";

function readStdin(): Promise<string> {
  return new Promise((resolve) => {
    if (process.stdin.isTTY) return resolve("");
    let buf = "";
    process.stdin.setEncoding("utf8");
    process.stdin.on("data", (d) => (buf += d));
    process.stdin.on("end", () => resolve(buf));
    process.stdin.on("error", () => resolve(buf));
  });
}

function parse(text: string): unknown {
  try {
    return JSON.parse(text);
  } catch {
    return null;
  }
}

/** Forward the statusLine JSON to the daemon, then run the user's original statusLine (if any). */
async function statusline(): Promise<void> {
  const input = await readStdin();
  const payload = parse(input);
  const forwarded = payload ? send({ kind: "statusline", payload }, 300) : Promise.resolve(null);

  const original = readConfig().originalStatusLine;
  if (original?.command) {
    const child = spawn(original.command, { shell: true, stdio: ["pipe", "inherit", "inherit"] });
    child.stdin.end(input);
    await Promise.all([forwarded, new Promise((r) => child.on("close", r))]);
  } else {
    const p = payload as { model?: { display_name?: string }; workspace?: { current_dir?: string } } | null;
    const dir = p?.workspace?.current_dir?.split("/").pop() ?? "";
    const model = p?.model?.display_name ?? "";
    process.stdout.write([model, dir].filter(Boolean).join(" · ") + "\n");
    await forwarded;
  }
}

async function hook(): Promise<void> {
  const payload = parse(await readStdin());
  if (payload) await send({ kind: "hook", payload }, 300);
}

async function status(): Promise<void> {
  const res = (await send({ kind: "status" }, 1000)) as
    | { panel: { port: string; fw: string | null } | null; state: { sessions: unknown[]; usage: unknown } }
    | null;
  if (!res) {
    console.log("daemon: not running");
    process.exitCode = 1;
    return;
  }
  console.log(`daemon: running`);
  console.log(`panel:  ${res.panel ? `${res.panel.port} (fw ${res.panel.fw ?? "?"})` : "not connected"}`);
  console.log(JSON.stringify(res.state, null, 2));
}

const USAGE = `ai-panel — Claude Code status panel companion

usage: ai-panel <command>

  install [--no-launchd]  add hooks + statusLine to ~/.claude/settings.json and start the daemon (launchd)
  uninstall               remove everything install added (restores your statusLine)
  daemon [-v]             run the daemon in the foreground
  status                  show daemon / panel / session state
  demo                    drive the panel with fake data (stop the daemon first)
  hook                    (internal) forward a hook event from stdin
  statusline              (internal) statusLine command
`;

async function main(): Promise<void> {
  const [cmd, ...args] = process.argv.slice(2);
  switch (cmd) {
    case "daemon":
      return runDaemon({ verbose: args.includes("-v") || args.includes("--verbose") });
    case "hook":
      return hook();
    case "statusline":
      return statusline();
    case "install":
      return install({ launchd: !args.includes("--no-launchd") });
    case "uninstall":
      return uninstall();
    case "status":
      return status();
    case "demo":
      return runDemo();
    default:
      process.stdout.write(USAGE);
      if (cmd && cmd !== "help" && cmd !== "--help" && cmd !== "-h") process.exitCode = 1;
  }
}

main().catch((e) => {
  // Hooks and statusLine must never break Claude Code.
  if (process.argv[2] === "hook" || process.argv[2] === "statusline") process.exit(0);
  console.error(e instanceof Error ? e.message : e);
  process.exit(1);
});
