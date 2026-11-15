// `ai-panel install|uninstall` — merge hooks + statusLine into ~/.claude/settings.json, set up launchd.
import { execFileSync } from "node:child_process";
import fs from "node:fs";
import path from "node:path";
import {
  CLAUDE_SETTINGS,
  CLI_PATH,
  HOOK_SCRIPT,
  LAUNCHD_LABEL,
  LAUNCHD_PLIST,
  LOG_PATH,
  SOCK_PATH,
  StatusLineConfig,
  ensureAppDir,
  readConfig,
  writeConfig,
} from "./paths.js";

/** Hook events we listen to. PreToolUse cannot be async, so it stays sync (the script is ~5 ms). */
export const HOOK_EVENTS = [
  "SessionStart",
  "SessionEnd",
  "UserPromptSubmit",
  "PreToolUse",
  "PostToolUse",
  "PostToolUseFailure",
  "PermissionRequest",
  "PermissionDenied",
  "Notification",
  "Stop",
  "StopFailure",
  "SubagentStart",
  "PreCompact",
  "Elicitation",
  "ElicitationResult",
] as const;

const SYNC_EVENTS = new Set<string>(["PreToolUse"]);
const MARKER = "/.ai-panel/hook.sh";

type HookHandler = { type: string; command?: string; async?: boolean; timeout?: number; [k: string]: unknown };
type MatcherGroup = { matcher?: string; hooks: HookHandler[]; [k: string]: unknown };
type Settings = {
  hooks?: Record<string, MatcherGroup[]>;
  statusLine?: StatusLineConfig;
  [k: string]: unknown;
};

const q = (s: string) => `"${s.replace(/(["\\$`])/g, "\\$1")}"`;

export function statusLineCommand(): string {
  return `${q(process.execPath)} ${q(CLI_PATH)} statusline`;
}

export function isOurStatusLine(sl: StatusLineConfig | undefined): boolean {
  return !!sl?.command && sl.command.includes("ai-panel") && /cli\.js"? statusline$/.test(sl.command);
}

/** Remove every hook handler we added; drop groups/events that become empty. */
export function stripHooks(settings: Settings): Settings {
  const out: Settings = { ...settings };
  if (!out.hooks) return out;
  const hooks: Record<string, MatcherGroup[]> = {};
  for (const [event, groups] of Object.entries(out.hooks)) {
    const kept = groups
      .map((g) => ({ ...g, hooks: (g.hooks ?? []).filter((h) => !h.command?.includes(MARKER)) }))
      .filter((g) => g.hooks.length > 0);
    if (kept.length) hooks[event] = kept;
  }
  if (Object.keys(hooks).length) out.hooks = hooks;
  else delete out.hooks;
  return out;
}

export function addHooks(settings: Settings, hookScript: string): Settings {
  const out = stripHooks(settings);
  const hooks = { ...(out.hooks ?? {}) };
  for (const event of HOOK_EVENTS) {
    const handler: HookHandler = { type: "command", command: q(hookScript), timeout: 5 };
    if (!SYNC_EVENTS.has(event)) handler.async = true;
    hooks[event] = [...(hooks[event] ?? []), { hooks: [handler] }];
  }
  out.hooks = hooks;
  return out;
}

/** Point statusLine at us; returns [newSettings, originalStatusLine-to-remember]. */
export function wrapStatusLine(
  settings: Settings,
  ourCommand: string,
  previous: StatusLineConfig | null | undefined,
): [Settings, StatusLineConfig | null] {
  const out = { ...settings };
  const current = out.statusLine;
  const original = current && !isOurStatusLine(current) ? current : (previous ?? null);
  out.statusLine = { type: "command", command: ourCommand, padding: original?.padding ?? 0 };
  return [out, original];
}

export function unwrapStatusLine(settings: Settings, original: StatusLineConfig | null | undefined): Settings {
  const out = { ...settings };
  if (isOurStatusLine(out.statusLine)) {
    if (original) out.statusLine = original;
    else delete out.statusLine;
  }
  return out;
}

const HOOK_SH = `#!/bin/sh
# ai-panel: forward a Claude Code hook event to the ai-panel daemon. Never blocks Claude.
# cd + relative path keeps us clear of the 104-byte unix socket path limit.
cd "${path.dirname(SOCK_PATH)}" 2>/dev/null || { cat >/dev/null; exit 0; }
[ -S "${path.basename(SOCK_PATH)}" ] || { cat >/dev/null; exit 0; }
cat | nc -U -w 1 "${path.basename(SOCK_PATH)}" >/dev/null 2>&1
exit 0
`;

function plist(): string {
  return `<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
  <key>Label</key><string>${LAUNCHD_LABEL}</string>
  <key>ProgramArguments</key>
  <array>
    <string>${process.execPath}</string>
    <string>${CLI_PATH}</string>
    <string>daemon</string>
  </array>
  <key>RunAtLoad</key><true/>
  <key>KeepAlive</key><true/>
  <key>StandardOutPath</key><string>${LOG_PATH}</string>
  <key>StandardErrorPath</key><string>${LOG_PATH}</string>
</dict>
</plist>
`;
}

function readSettings(): Settings {
  try {
    return JSON.parse(fs.readFileSync(CLAUDE_SETTINGS, "utf8")) as Settings;
  } catch (e) {
    if ((e as NodeJS.ErrnoException).code === "ENOENT") return {};
    throw new Error(`cannot parse ${CLAUDE_SETTINGS}: ${(e as Error).message}`);
  }
}

function writeSettings(s: Settings): void {
  fs.mkdirSync(path.dirname(CLAUDE_SETTINGS), { recursive: true });
  if (fs.existsSync(CLAUDE_SETTINGS)) {
    const backup = `${CLAUDE_SETTINGS}.ai-panel-backup-${Date.now()}`;
    fs.copyFileSync(CLAUDE_SETTINGS, backup);
    console.log(`backed up settings to ${backup}`);
  }
  fs.writeFileSync(CLAUDE_SETTINGS, JSON.stringify(s, null, 2) + "\n");
}

function launchctl(args: string[]): void {
  try {
    execFileSync("launchctl", args, { stdio: "ignore" });
  } catch {
    /* already (un)loaded */
  }
}

export function install(opts: { launchd: boolean }): void {
  ensureAppDir();
  fs.writeFileSync(HOOK_SCRIPT, HOOK_SH, { mode: 0o755 });

  const cfg = readConfig();
  let settings = addHooks(readSettings(), HOOK_SCRIPT);
  const [next, original] = wrapStatusLine(settings, statusLineCommand(), cfg.originalStatusLine);
  settings = next;
  writeSettings(settings);
  writeConfig({ ...cfg, originalStatusLine: original });
  console.log(`installed hooks + statusLine in ${CLAUDE_SETTINGS}`);
  if (original) console.log(`your previous statusLine is chained: ${original.command}`);

  if (opts.launchd && process.platform === "darwin") {
    fs.mkdirSync(path.dirname(LAUNCHD_PLIST), { recursive: true });
    fs.writeFileSync(LAUNCHD_PLIST, plist());
    const domain = `gui/${process.getuid?.() ?? 501}`;
    launchctl(["bootout", domain, LAUNCHD_PLIST]);
    launchctl(["bootstrap", domain, LAUNCHD_PLIST]);
    console.log(`daemon installed as launchd agent ${LAUNCHD_LABEL} (log: ${LOG_PATH})`);
  } else {
    console.log("start the daemon with: ai-panel daemon");
  }
}

export function uninstall(): void {
  const cfg = readConfig();
  let settings = stripHooks(readSettings());
  settings = unwrapStatusLine(settings, cfg.originalStatusLine);
  writeSettings(settings);
  console.log(`removed ai-panel hooks/statusLine from ${CLAUDE_SETTINGS}`);
  if (process.platform === "darwin" && fs.existsSync(LAUNCHD_PLIST)) {
    launchctl(["bootout", `gui/${process.getuid?.() ?? 501}`, LAUNCHD_PLIST]);
    fs.unlinkSync(LAUNCHD_PLIST);
    console.log("removed launchd agent");
  }
}
