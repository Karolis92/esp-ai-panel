import fs from "node:fs";
import os from "node:os";
import path from "node:path";
import { fileURLToPath } from "node:url";

export const HOME = os.homedir();
export const APP_DIR = process.env.AI_PANEL_DIR ?? path.join(HOME, ".ai-panel");
export const SOCK_PATH = path.join(APP_DIR, "sock");
export const CONFIG_PATH = path.join(APP_DIR, "config.json");
export const HOOK_SCRIPT = path.join(APP_DIR, "hook.sh");
export const LOG_PATH = path.join(APP_DIR, "daemon.log");
export const CLAUDE_SETTINGS = path.join(HOME, ".claude", "settings.json");
export const LAUNCHD_LABEL = "gg.ai-panel";
export const LAUNCHD_PLIST = path.join(HOME, "Library", "LaunchAgents", `${LAUNCHD_LABEL}.plist`);

/** Absolute path of the compiled CLI entry (dist/cli.js). */
export const CLI_PATH = path.join(path.dirname(fileURLToPath(import.meta.url)), "cli.js");

export interface StatusLineConfig {
  type: string;
  command: string;
  padding?: number;
  [k: string]: unknown;
}

export interface Config {
  /** Serial port path override, e.g. /dev/cu.usbserial-110 */
  port?: string;
  baud?: number;
  /** The user's statusLine before `ai-panel install`, chained by `ai-panel statusline`. */
  originalStatusLine?: StatusLineConfig | null;
}

export function ensureAppDir(): void {
  fs.mkdirSync(APP_DIR, { recursive: true, mode: 0o700 });
}

export function readConfig(): Config {
  try {
    return JSON.parse(fs.readFileSync(CONFIG_PATH, "utf8")) as Config;
  } catch {
    return {};
  }
}

export function writeConfig(cfg: Config): void {
  ensureAppDir();
  fs.writeFileSync(CONFIG_PATH, JSON.stringify(cfg, null, 2) + "\n");
}
