import { describe, expect, it } from "vitest";
import { HOOK_EVENTS, addHooks, stripHooks, unwrapStatusLine, wrapStatusLine } from "../src/install.js";

const HOOK = "/Users/me/.ai-panel/hook.sh";
const OURS = `"/usr/local/bin/node" "/opt/ai-panel/dist/cli.js" statusline`;

describe("settings merge", () => {
  const userSettings = {
    theme: "dark",
    hooks: {
      Stop: [{ hooks: [{ type: "command", command: "say done" }] }],
      PreToolUse: [{ matcher: "Bash", hooks: [{ type: "command", command: "guard.sh" }] }],
    },
    statusLine: { type: "command", command: "~/my-status.sh", padding: 1 },
  };

  it("adds our hooks alongside the user's, idempotently", () => {
    const once = addHooks(userSettings, HOOK);
    const twice = addHooks(once, HOOK);
    expect(twice).toEqual(once);
    for (const ev of HOOK_EVENTS) expect(once.hooks![ev].length).toBeGreaterThan(0);
    expect(once.hooks!.Stop).toHaveLength(2);
    expect(once.hooks!.Stop[0].hooks[0].command).toBe("say done");
    const ours = once.hooks!.Stop[1].hooks[0];
    expect(ours).toMatchObject({ type: "command", async: true });
    expect(once.hooks!.PreToolUse[1].hooks[0].async).toBeUndefined();
    expect(once.theme).toBe("dark");
  });

  it("strip restores the user's hooks exactly", () => {
    expect(stripHooks(addHooks(userSettings, HOOK))).toEqual(userSettings);
    expect(stripHooks(addHooks({}, HOOK))).toEqual({});
  });

  it("wraps and restores the statusLine", () => {
    const [wrapped, original] = wrapStatusLine(userSettings, OURS, null);
    expect(original).toEqual(userSettings.statusLine);
    expect(wrapped.statusLine).toEqual({ type: "command", command: OURS, padding: 1 });
    // Re-install keeps the remembered original rather than recording ourselves.
    const [again, original2] = wrapStatusLine(wrapped, OURS, original);
    expect(original2).toEqual(userSettings.statusLine);
    expect(again.statusLine).toEqual(wrapped.statusLine);
    expect(unwrapStatusLine(again, original2)).toEqual(userSettings);
  });

  it("removes the statusLine on uninstall when there was none", () => {
    const [wrapped, original] = wrapStatusLine({}, OURS, null);
    expect(original).toBeNull();
    expect(unwrapStatusLine(wrapped, original)).toEqual({});
  });
});
