// Pure session state machine: Claude Code hook events + statusLine snapshots → display state.
import path from "node:path";
import {
  Anim,
  CardState,
  MAX_ACTIVITY,
  MAX_SESSIONS,
  MAX_TITLE,
  StateMsg,
  WireUsage,
  WireUsageWindow,
} from "./protocol.js";

/** A Claude Code hook payload (only the fields we use). */
export interface HookEvent {
  hook_event_name: string;
  session_id: string;
  cwd?: string;
  permission_mode?: string;
  tool_name?: string;
  tool_input?: Record<string, unknown>;
  notification_type?: string;
  message?: string;
  source?: string;
  reason?: string;
  [k: string]: unknown;
}

/** The statusLine JSON payload (only the fields we use). */
export interface StatusLineInput {
  session_id?: string;
  cwd?: string;
  workspace?: { current_dir?: string; project_dir?: string };
  rate_limits?: {
    five_hour?: { used_percentage?: number; resets_at?: number } | null;
    seven_day?: { used_percentage?: number; resets_at?: number } | null;
  } | null;
  [k: string]: unknown;
}

export interface Session {
  sessionId: string;
  title: string;
  activity: string;
  state: CardState;
  anim: Anim;
  /** ms epoch of first sighting (stable card order) */
  createdAt: number;
  /** ms epoch of last event */
  updatedAt: number;
  /** ms epoch when a "done" card falls back to idle */
  doneUntil?: number;
  /** ms epoch when the visible scene last changed (for the minimum hold time) */
  shownAt: number;
  /** a working→working scene change waiting for the hold time to pass */
  pending?: Scene;
  /** tool currently running (PreToolUse … PostToolUse) */
  toolUseId?: string;
  toolStartedAt?: number;
}

interface Scene {
  state: CardState;
  anim: Anim;
  activity: string;
}

export const DONE_HOLD_MS = 60_000;
/** Minimum time a working scene stays up, so rapid tool calls don't flicker. */
export const SCENE_HOLD_MS = 2_000;
/** A command running longer than this switches from terminal to coffee. */
export const LONG_COMMAND_MS = 10_000;
export const STALE_MS = 2 * 60 * 60_000;

const EDIT_TOOLS = new Set(["Edit", "Write", "MultiEdit", "NotebookEdit"]);
const READ_TOOLS = new Set(["Read", "Grep", "Glob", "WebFetch", "WebSearch", "LS", "ToolSearch"]);
const AGENT_TOOLS = new Set(["Task", "Agent"]);
const ASK_TOOLS = new Set(["AskUserQuestion"]);

export function clip(s: string, max: number): string {
  const one = s.replace(/\s+/g, " ").trim();
  return one.length <= max ? one : one.slice(0, max - 1) + "…";
}

export function titleFromCwd(cwd: string | undefined): string {
  if (!cwd) return "CLAUDE";
  const base = path.basename(cwd.replace(/[\\/]+$/, "")) || cwd;
  return clip(base.toUpperCase(), MAX_TITLE);
}

function str(v: unknown): string | undefined {
  return typeof v === "string" && v.length > 0 ? v : undefined;
}

/** Describe a tool call: [anim, activity text]. */
export function describeTool(
  tool: string,
  input: Record<string, unknown> | undefined,
  planMode: boolean,
): [Anim, string] {
  const inp = input ?? {};
  if (EDIT_TOOLS.has(tool)) {
    const f = str(inp.file_path) ?? str(inp.notebook_path);
    return ["laptop", f ? `Editing ${path.basename(f)}` : "Editing…"];
  }
  if (planMode) return ["plan", "Planning…"];
  if (READ_TOOLS.has(tool)) {
    if (tool === "WebSearch" || tool === "WebFetch") return ["book", "Browsing web…"];
    return ["book", "Researching…"];
  }
  if (tool === "Bash") {
    const words = (str(inp.command) ?? "").trim().split(/\s+/).filter(Boolean);
    if (!words.length) return ["terminal", "Running…"];
    const two = `Running ${words.slice(0, 2).join(" ")}`;
    return ["terminal", two.length <= MAX_ACTIVITY ? two : `Running ${words[0]}`];
  }
  if (AGENT_TOOLS.has(tool)) return ["thinking", "Delegating…"];
  if (tool.startsWith("mcp__")) {
    const parts = tool.split("__");
    return ["laptop", `Using ${parts[1] ?? "MCP"}`];
  }
  return ["laptop", "Working…"];
}

export class SessionStore {
  private sessions = new Map<string, Session>();
  private usage: WireUsage = { s5h: null, wk: null };

  constructor(private clock: () => number = Date.now) {}

  list(): Session[] {
    return [...this.sessions.values()];
  }

  get(id: string): Session | undefined {
    return this.sessions.get(id);
  }

  getUsage(): WireUsage {
    return this.usage;
  }

  private ensure(id: string, cwd: string | undefined): Session {
    let s = this.sessions.get(id);
    if (!s) {
      s = {
        sessionId: id,
        title: titleFromCwd(cwd),
        activity: "Waiting…",
        state: "idle",
        anim: "sleep",
        createdAt: this.clock(),
        updatedAt: this.clock(),
        shownAt: 0,
      };
      this.sessions.set(id, s);
    } else if (s.title === "CLAUDE" && cwd) {
      s.title = titleFromCwd(cwd);
    }
    return s;
  }

  /** Request a scene. Working→working changes wait until the current scene has been up for
   *  SCENE_HOLD_MS; anything else (needs you, done, idle, error) shows immediately. */
  private set(s: Session, state: CardState, anim: Anim, activity: string): void {
    const scene: Scene = { state, anim, activity: clip(activity, MAX_ACTIVITY) };
    const held = s.state === "working" && state === "working" && this.clock() - s.shownAt < SCENE_HOLD_MS;
    if (held) {
      s.pending = scene;
    } else {
      this.show(s, scene);
    }
  }

  private show(s: Session, scene: Scene): void {
    s.pending = undefined;
    if (s.state !== scene.state || s.anim !== scene.anim || s.activity !== scene.activity) {
      s.shownAt = this.clock();
    }
    s.state = scene.state;
    s.anim = scene.anim;
    s.activity = scene.activity;
    s.doneUntil = scene.state === "done" ? this.clock() + DONE_HOLD_MS : undefined;
  }

  private thinking(s: Session, planMode: boolean): void {
    if (planMode) this.set(s, "working", "plan", "Planning…");
    else this.set(s, "working", "thinking", "Thinking…");
  }

  /** Apply one hook event. Returns true if visible state changed. */
  applyHook(ev: HookEvent): boolean {
    if (!ev || typeof ev.session_id !== "string" || typeof ev.hook_event_name !== "string") {
      return false;
    }
    const before = this.fingerprint();
    const id = ev.session_id;
    const planMode = ev.permission_mode === "plan";

    if (ev.hook_event_name === "SessionEnd") {
      // "clear" starts a fresh session id right after; any other reason means the session is gone.
      this.sessions.delete(id);
      return before !== this.fingerprint();
    }

    const s = this.ensure(id, ev.cwd);
    s.updatedAt = this.clock();

    switch (ev.hook_event_name) {
      case "SessionStart":
        this.set(s, "idle", "sleep", "Waiting…");
        break;

      case "UserPromptSubmit":
        s.toolUseId = undefined;
        this.thinking(s, planMode);
        break;

      case "PreToolUse": {
        const tool = ev.tool_name ?? "";
        if (ASK_TOOLS.has(tool)) {
          this.set(s, "needs", "alert", "Has a question");
        } else if (tool === "ExitPlanMode") {
          this.set(s, "needs", "alert", "Plan ready");
        } else {
          const [anim, text] = describeTool(tool, ev.tool_input, planMode);
          this.set(s, "working", anim, text);
        }
        s.toolUseId = str(ev.tool_use_id);
        s.toolStartedAt = this.clock();
        break;
      }

      case "PermissionRequest":
        this.set(s, "needs", "alert", "Needs you");
        break;

      case "Notification":
        if (ev.notification_type === "permission_prompt" || ev.notification_type === "elicitation_dialog") {
          this.set(s, "needs", "alert", "Needs you");
        } else if (ev.notification_type === "idle_prompt") {
          this.set(s, "idle", "sleep", "Waiting…");
        }
        break;

      case "Elicitation":
        this.set(s, "needs", "alert", "Needs you");
        break;

      case "PostToolUse":
      case "PostToolUseFailure": {
        // Async hooks can arrive out of order: only the tool that is still current may end the scene.
        const id = str(ev.tool_use_id);
        if (id && s.toolUseId && id !== s.toolUseId) break;
        s.toolUseId = undefined;
        if (s.state === "working" || s.state === "needs") this.thinking(s, planMode);
        break;
      }

      case "PermissionDenied":
      case "ElicitationResult":
        if (s.state === "needs") this.thinking(s, planMode);
        break;

      case "SubagentStart":
        if (s.state !== "needs") this.set(s, "working", "thinking", "Delegating…");
        break;

      case "PreCompact":
        if (s.state !== "needs") this.set(s, "working", "book", "Compacting…");
        break;

      case "Stop":
        this.set(s, "done", "done", "Done");
        break;

      case "StopFailure":
        this.set(s, "error", "error", "Error");
        break;

      default:
        break;
    }
    return before !== this.fingerprint();
  }

  /** Apply a statusLine snapshot (rate limits + session discovery). Returns true if visible state changed. */
  applyStatusLine(input: StatusLineInput): boolean {
    const before = this.fingerprint();
    if (input.session_id) {
      const cwd = input.workspace?.project_dir ?? input.workspace?.current_dir ?? input.cwd;
      this.ensure(input.session_id, cwd);
    }
    const rl = input.rate_limits;
    if (rl) {
      const conv = (w: { used_percentage?: number; resets_at?: number } | null | undefined): WireUsageWindow | null => {
        if (!w || typeof w.used_percentage !== "number") return null;
        return {
          pct: Math.max(0, Math.min(100, Math.round(w.used_percentage))),
          reset: typeof w.resets_at === "number" ? Math.round(w.resets_at) : null,
        };
      };
      // Each window can be absent independently; keep the previous value when it is.
      const s5h = conv(rl.five_hour);
      const wk = conv(rl.seven_day);
      this.usage = { s5h: s5h ?? this.usage.s5h, wk: wk ?? this.usage.wk };
    }
    return before !== this.fingerprint();
  }

  /** Time-based transitions: held scenes, terminal → coffee, done → idle, prune stale sessions. */
  tick(): boolean {
    const now = this.clock();
    let changed = false;
    for (const [id, s] of this.sessions) {
      if (s.pending && now - s.shownAt >= SCENE_HOLD_MS) {
        this.show(s, s.pending);
        changed = true;
      }
      if (s.state === "working" && s.anim === "terminal" && s.toolUseId !== undefined &&
          s.toolStartedAt !== undefined && now - s.toolStartedAt >= LONG_COMMAND_MS && !s.pending) {
        const shownAt = s.shownAt; // same command continuing: don't restart the hold timer
        this.show(s, { state: "working", anim: "coffee", activity: s.activity });
        s.shownAt = shownAt;
        changed = true;
      }
      if (s.state === "done" && s.doneUntil !== undefined && now >= s.doneUntil) {
        this.show(s, { state: "idle", anim: "sleep", activity: "Waiting…" });
        changed = true;
      }
      if (now - s.updatedAt > STALE_MS) {
        this.sessions.delete(id);
        changed = true;
      }
    }
    return changed;
  }

  snapshot(): StateMsg {
    // Cards needing attention first; everything else keeps a stable (creation) order so
    // cards don't jump around while sessions flip between working and idle.
    const rank = (s: Session) => (s.state === "needs" ? 0 : 1);
    const sorted = this.list().sort(
      (a, b) => rank(a) - rank(b) || a.createdAt - b.createdAt || a.sessionId.localeCompare(b.sessionId),
    );
    return {
      t: "state",
      now: Math.floor(this.clock() / 1000),
      sessions: sorted.slice(0, MAX_SESSIONS).map((s) => ({
        id: s.sessionId.slice(0, 8),
        title: s.title,
        activity: s.activity,
        state: s.state,
        anim: s.anim,
      })),
      usage: this.usage,
    };
  }

  private fingerprint(): string {
    const { now: _now, ...rest } = this.snapshot();
    return JSON.stringify(rest);
  }
}
