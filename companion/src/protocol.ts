// Wire protocol types — see ../../protocol/PROTOCOL.md

export type CardState = "working" | "needs" | "idle" | "done" | "error";
export type Anim =
  | "thinking" // thought bubble
  | "laptop" // editing files
  | "terminal" // running a command
  | "coffee" // a command that has been running a while
  | "book" // reading / searching
  | "plan" // writing in a notepad (plan mode)
  | "sleep"
  | "alert"
  | "done"
  | "error";

export interface WireSession {
  id: string;
  title: string;
  activity: string;
  state: CardState;
  anim: Anim;
}

export interface WireUsageWindow {
  pct: number;
  reset: number | null;
}

export interface WireUsage {
  s5h: WireUsageWindow | null;
  wk: WireUsageWindow | null;
}

export interface StateMsg {
  t: "state";
  now: number;
  sessions: WireSession[];
  usage: WireUsage;
}

export interface PingMsg {
  t: "ping";
  now: number;
}

export type HostMsg = StateMsg | PingMsg;

export type DeviceMsg =
  | { t: "hello"; fw: string }
  | { t: "tap"; id: string };

export const MAX_SESSIONS = 18;
export const MAX_TITLE = 24; // long titles scroll on the device
export const MAX_ACTIVITY = 32; // longer text scrolls on the device
export const MAX_LINE = 4096;

/** Serialize a host message as one NDJSON line, trimming sessions if the line would be too long. */
export function encode(msg: HostMsg): string {
  let line = JSON.stringify(msg);
  if (msg.t === "state") {
    const m = { ...msg, sessions: [...msg.sessions] };
    while (line.length + 1 > MAX_LINE && m.sessions.length > 0) {
      m.sessions.pop();
      line = JSON.stringify(m);
    }
  }
  return line + "\n";
}
