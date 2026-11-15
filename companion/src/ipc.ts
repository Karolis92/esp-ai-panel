// Unix-socket IPC between short-lived hook/statusline processes and the daemon.
// Each connection carries exactly one JSON object; the daemon replies (optionally) and closes.
import fs from "node:fs";
import net from "node:net";
import { SOCK_PATH, ensureAppDir } from "./paths.js";

export type IpcRequest =
  | { kind: "hook"; payload: unknown }
  | { kind: "statusline"; payload: unknown }
  | { kind: "status" };

export type IpcHandler = (req: IpcRequest) => unknown;

/** Parse a request; also accepts a raw hook payload (anything with hook_event_name). */
export function parseRequest(text: string): IpcRequest | null {
  let obj: unknown;
  try {
    obj = JSON.parse(text);
  } catch {
    return null;
  }
  if (!obj || typeof obj !== "object") return null;
  const o = obj as Record<string, unknown>;
  if (o.kind === "hook" || o.kind === "statusline") return { kind: o.kind, payload: o.payload };
  if (o.kind === "status") return { kind: "status" };
  if (typeof o.hook_event_name === "string") return { kind: "hook", payload: o };
  return null;
}

export function startServer(handler: IpcHandler, sockPath = SOCK_PATH): net.Server {
  ensureAppDir();
  try {
    fs.unlinkSync(sockPath);
  } catch {
    /* not there */
  }
  const server = net.createServer((sock) => {
    let buf = "";
    let done = false;
    const finish = () => {
      if (done) return;
      const req = parseRequest(buf);
      if (!req) return;
      done = true;
      let reply: unknown;
      try {
        reply = handler(req);
      } catch (e) {
        reply = { error: String(e) };
      }
      sock.end(reply === undefined ? "" : JSON.stringify(reply) + "\n");
    };
    sock.setEncoding("utf8");
    sock.setTimeout(3000, () => sock.destroy());
    sock.on("data", (d: string) => {
      buf += d;
      if (buf.length > 1_000_000) sock.destroy();
      else finish(); // the payload may arrive without a half-close (e.g. `nc -U`)
    });
    sock.on("end", finish);
    sock.on("error", () => {});
  });
  server.listen(sockPath, () => {
    try {
      fs.chmodSync(sockPath, 0o600);
    } catch {
      /* ignore */
    }
  });
  return server;
}

/** Send one request; resolves with the parsed reply (or null). Never rejects. */
export function send(req: IpcRequest, timeoutMs = 500, sockPath = SOCK_PATH): Promise<unknown> {
  return new Promise((resolve) => {
    let buf = "";
    let settled = false;
    const done = (v: unknown) => {
      if (!settled) {
        settled = true;
        resolve(v);
      }
    };
    const sock = net.createConnection(sockPath);
    sock.setEncoding("utf8");
    const timer = setTimeout(() => {
      sock.destroy();
      done(null);
    }, timeoutMs);
    sock.on("connect", () => sock.end(JSON.stringify(req)));
    sock.on("data", (d: string) => (buf += d));
    sock.on("close", () => {
      clearTimeout(timer);
      try {
        done(buf.trim() ? JSON.parse(buf) : null);
      } catch {
        done(null);
      }
    });
    sock.on("error", () => {
      clearTimeout(timer);
      done(null);
    });
  });
}
