import { describe, expect, it } from "vitest";
import { DONE_HOLD_MS, HookEvent, LONG_COMMAND_MS, SCENE_HOLD_MS, STALE_MS, SessionStore, titleFromCwd } from "../src/sessions.js";
import { encode, MAX_LINE } from "../src/protocol.js";

function setup() {
  let now = 1_700_000_000_000;
  const clock = () => now;
  const store = new SessionStore(clock);
  const advance = (ms: number) => (now += ms);
  const ev = (name: string, extra: Partial<HookEvent> = {}, id = "sess-aaaaaaaa"): HookEvent => ({
    hook_event_name: name,
    session_id: id,
    cwd: "/Users/me/src/esp-ai-panel",
    permission_mode: "default",
    ...extra,
  });
  return { store, advance, ev };
}

describe("titleFromCwd", () => {
  it("uses the uppercased folder name, clipped to 24", () => {
    expect(titleFromCwd("/a/b/esp-ai-panel/")).toBe("ESP-AI-PANEL");
    expect(titleFromCwd("/x/a-really-long-project-name")).toBe("A-REALLY-LONG-PROJECT-N…");
    expect(titleFromCwd(undefined)).toBe("CLAUDE");
  });
});

describe("SessionStore", () => {
  it("walks a typical session lifecycle", () => {
    const { store, ev, advance } = setup();
    store.applyHook(ev("SessionStart", { source: "startup" }));
    expect(store.snapshot().sessions[0]).toMatchObject({ title: "ESP-AI-PANEL", state: "idle", anim: "sleep" });

    store.applyHook(ev("UserPromptSubmit", { prompt: "fix it" }));
    expect(store.snapshot().sessions[0]).toMatchObject({ state: "working", anim: "thinking", activity: "Thinking…" });

    advance(SCENE_HOLD_MS);
    store.applyHook(ev("PreToolUse", { tool_name: "Edit", tool_input: { file_path: "/x/main.c" } }));
    expect(store.snapshot().sessions[0]).toMatchObject({ anim: "laptop", activity: "Editing main.c" });

    advance(SCENE_HOLD_MS);
    store.applyHook(ev("PreToolUse", { tool_name: "Bash", tool_input: { command: "npm test -- --watch" } }));
    expect(store.snapshot().sessions[0]).toMatchObject({ anim: "terminal", activity: "Running npm test" });

    advance(SCENE_HOLD_MS);
    store.applyHook(ev("PreToolUse", { tool_name: "Grep", tool_input: {} }));
    expect(store.snapshot().sessions[0]).toMatchObject({ anim: "book", activity: "Researching…" });

    store.applyHook(ev("PermissionRequest", { tool_name: "Bash" }));
    expect(store.snapshot().sessions[0]).toMatchObject({ state: "needs", anim: "alert" }); // never held

    store.applyHook(ev("PostToolUse", { tool_name: "Bash" }));
    expect(store.snapshot().sessions[0]).toMatchObject({ state: "working", anim: "thinking" });

    store.applyHook(ev("Stop"));
    expect(store.snapshot().sessions[0]).toMatchObject({ state: "done", activity: "Done" });
    advance(DONE_HOLD_MS - 1);
    expect(store.tick()).toBe(false);
    advance(1);
    expect(store.tick()).toBe(true);
    expect(store.snapshot().sessions[0]).toMatchObject({ state: "idle", activity: "Waiting…" });

    store.applyHook(ev("SessionEnd", { reason: "prompt_input_exit" }));
    expect(store.snapshot().sessions).toHaveLength(0);
  });

  it("shows the notepad in plan mode, but still shows edits", () => {
    const { store, ev, advance } = setup();
    store.applyHook(ev("UserPromptSubmit", { permission_mode: "plan" }));
    expect(store.snapshot().sessions[0]).toMatchObject({ anim: "plan", activity: "Planning…" });
    store.applyHook(ev("PreToolUse", { permission_mode: "plan", tool_name: "Read" }));
    expect(store.snapshot().sessions[0].anim).toBe("plan");
    advance(SCENE_HOLD_MS);
    store.applyHook(ev("PreToolUse", { permission_mode: "plan", tool_name: "Write", tool_input: { file_path: "/p/plan.md" } }));
    expect(store.snapshot().sessions[0]).toMatchObject({ anim: "laptop", activity: "Editing plan.md" });
  });

  it("flags questions, plan approval and permission notifications as needs", () => {
    const { store, ev } = setup();
    store.applyHook(ev("PreToolUse", { tool_name: "AskUserQuestion" }));
    expect(store.snapshot().sessions[0].state).toBe("needs");
    store.applyHook(ev("PostToolUse", { tool_name: "AskUserQuestion" }));
    store.applyHook(ev("PreToolUse", { tool_name: "ExitPlanMode" }));
    expect(store.snapshot().sessions[0]).toMatchObject({ state: "needs", activity: "Plan ready" });
    store.applyHook(ev("UserPromptSubmit"));
    store.applyHook(ev("Notification", { notification_type: "permission_prompt" }));
    expect(store.snapshot().sessions[0].state).toBe("needs");
    store.applyHook(ev("Notification", { notification_type: "idle_prompt" }));
    expect(store.snapshot().sessions[0].state).toBe("idle");
  });

  it("sorts needs first and keeps a stable order otherwise", () => {
    const { store, ev, advance } = setup();
    store.applyHook(ev("SessionStart", { cwd: "/a/one" }, "s1"));
    advance(10);
    store.applyHook(ev("SessionStart", { cwd: "/a/two" }, "s2"));
    advance(10);
    store.applyHook(ev("SessionStart", { cwd: "/a/three" }, "s3"));
    advance(10);
    store.applyHook(ev("UserPromptSubmit", {}, "s1"));
    expect(store.snapshot().sessions.map((s) => s.title)).toEqual(["ONE", "TWO", "THREE"]);
    store.applyHook(ev("PermissionRequest", {}, "s3"));
    expect(store.snapshot().sessions.map((s) => s.title)).toEqual(["THREE", "ONE", "TWO"]);
  });

  it("holds working scenes for SCENE_HOLD_MS so rapid tools don't flicker", () => {
    const { store, ev, advance } = setup();
    store.applyHook(ev("UserPromptSubmit"));
    store.applyHook(ev("PreToolUse", { tool_name: "Read", tool_use_id: "a" }));
    store.applyHook(ev("PostToolUse", { tool_name: "Read", tool_use_id: "a" }));
    store.applyHook(ev("PreToolUse", { tool_name: "Edit", tool_use_id: "b", tool_input: { file_path: "/x/y.ts" } }));
    expect(store.snapshot().sessions[0].anim).toBe("thinking"); // still holding the first scene
    advance(SCENE_HOLD_MS - 1);
    expect(store.tick()).toBe(false);
    advance(1);
    expect(store.tick()).toBe(true); // latest pending scene wins
    expect(store.snapshot().sessions[0]).toMatchObject({ anim: "laptop", activity: "Editing y.ts" });
  });

  it("turns a long-running command into coffee", () => {
    const { store, ev, advance } = setup();
    store.applyHook(ev("SessionStart"));
    store.applyHook(ev("PreToolUse", { tool_name: "Bash", tool_use_id: "t1", tool_input: { command: "cargo build" } }));
    expect(store.snapshot().sessions[0].anim).toBe("terminal");
    advance(LONG_COMMAND_MS - 1);
    expect(store.tick()).toBe(false);
    advance(1);
    expect(store.tick()).toBe(true);
    expect(store.snapshot().sessions[0]).toMatchObject({ anim: "coffee", activity: "Running cargo build" });
    store.applyHook(ev("PostToolUse", { tool_name: "Bash", tool_use_id: "t1" }));
    expect(store.snapshot().sessions[0].anim).toBe("thinking");
  });

  it("ignores a late PostToolUse for a tool that is no longer current", () => {
    const { store, ev, advance } = setup();
    store.applyHook(ev("SessionStart"));
    store.applyHook(ev("PreToolUse", { tool_name: "Read", tool_use_id: "old" }));
    advance(SCENE_HOLD_MS);
    store.applyHook(ev("PreToolUse", { tool_name: "Bash", tool_use_id: "new", tool_input: { command: "ls" } }));
    store.applyHook(ev("PostToolUse", { tool_name: "Read", tool_use_id: "old" })); // arrives late
    expect(store.snapshot().sessions[0].anim).toBe("terminal");
  });

  it("prunes stale sessions and reports changes only when visible state changes", () => {
    const { store, ev, advance } = setup();
    expect(store.applyHook(ev("SessionStart"))).toBe(true);
    expect(store.applyHook(ev("SessionStart"))).toBe(false);
    expect(store.applyHook({} as HookEvent)).toBe(false);
    advance(STALE_MS + 1);
    expect(store.tick()).toBe(true);
    expect(store.snapshot().sessions).toHaveLength(0);
  });

  it("takes usage from statusLine rate_limits and discovers sessions", () => {
    const { store } = setup();
    store.applyStatusLine({
      session_id: "s9",
      workspace: { current_dir: "/x/api", project_dir: "/x/billing" },
      rate_limits: {
        five_hour: { used_percentage: 45.4, resets_at: 1_700_008_100 },
        seven_day: { used_percentage: 28, resets_at: 1_700_400_000 },
      },
    });
    const snap = store.snapshot();
    expect(snap.usage).toEqual({ s5h: { pct: 45, reset: 1_700_008_100 }, wk: { pct: 28, reset: 1_700_400_000 } });
    expect(snap.sessions[0]).toMatchObject({ title: "BILLING", state: "idle" });

    // A window missing from a later snapshot keeps its previous value.
    store.applyStatusLine({ rate_limits: { five_hour: { used_percentage: 50, resets_at: 1 } } });
    expect(store.snapshot().usage.wk?.pct).toBe(28);
    expect(store.snapshot().usage.s5h?.pct).toBe(50);
  });
});

describe("encode", () => {
  it("keeps lines under the device limit", () => {
    const { store, ev } = setup();
    for (let i = 0; i < 18; i++) {
      store.applyHook(ev("PreToolUse", { cwd: `/p/${"x".repeat(40)}${i}`, tool_name: "Bash", tool_input: { command: "y".repeat(200) } }, `id-${i}`));
    }
    const line = encode(store.snapshot());
    expect(line.endsWith("\n")).toBe(true);
    expect(line.length).toBeLessThanOrEqual(MAX_LINE);
    expect(JSON.parse(line).sessions.length).toBe(18);
  });
});
