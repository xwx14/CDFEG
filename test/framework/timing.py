"""回归测试耗时统计：SQLite 持久化 + 性能回归检测。纯标准库 sqlite3。"""
import sqlite3
from pathlib import Path
import subprocess


def detect_regress(now, last, threshold):
    """判定性能回归。

    last 为 None 或 <=0 时不判（首跑/无历史），返回 (False, "")。
    否则当 now > last*(1+threshold) 时判为回归，返回 (True, 说明文本)。
    """
    if last is None or last <= 0:
        return (False, "")
    boundary = last * (1.0 + threshold)
    if now > boundary:
        pct = (now - last) / last * 100.0
        return (True, f"上次 {last:.2f}s → 本次 {now:.2f}s (+{pct:.0f}%)")
    return (False, "")


def git_short_commit():
    """返回 git HEAD short sha；取不到（非 git 仓库/无 git/失败）返回 None。"""
    try:
        r = subprocess.run(
            ["git", "rev-parse", "--short", "HEAD"],
            capture_output=True, text=True, timeout=5,
        )
    except (FileNotFoundError, subprocess.TimeoutExpired):
        return None
    if r.returncode != 0:
        return None
    sha = r.stdout.strip()
    return sha or None


class TimingDb:
    """timing 记录的 SQLite 持久化。纯标准库 sqlite3。"""

    def __init__(self, db_path):
        self.db_path = str(db_path)

    def init(self):
        Path(self.db_path).parent.mkdir(parents=True, exist_ok=True)
        with sqlite3.connect(self.db_path) as conn:
            conn.execute(
                "CREATE TABLE IF NOT EXISTS timing ("
                " id INTEGER PRIMARY KEY AUTOINCREMENT,"
                " run_id TEXT NOT NULL,"
                " ts TEXT NOT NULL,"
                " case_name TEXT NOT NULL,"
                " suite TEXT NOT NULL,"
                " status TEXT NOT NULL,"
                " secs REAL NOT NULL,"
                " git_commit TEXT)"
            )
            conn.execute(
                "CREATE INDEX IF NOT EXISTS idx_case_ts ON timing(case_name, ts)"
            )

    def insert(self, result, run_id, ts, git_commit):
        with sqlite3.connect(self.db_path) as conn:
            conn.execute(
                "INSERT INTO timing (run_id, ts, case_name, suite, status, secs, git_commit)"
                " VALUES (?, ?, ?, ?, ?, ?, ?)",
                (run_id, ts, result.name, result.suite, result.status,
                 result.secs, git_commit),
            )

    def last_pass_secs(self, case_name):
        """同 case 最近一条 status='pass' 的 secs；无则 None。按 id DESC 取最新。"""
        with sqlite3.connect(self.db_path) as conn:
            row = conn.execute(
                "SELECT secs FROM timing WHERE case_name=? AND status='pass'"
                " ORDER BY id DESC LIMIT 1",
                (case_name,),
            ).fetchone()
        return row[0] if row else None

    def list_history(self, case_name):
        """返回 [(ts, status, secs, git_commit), ...]，按 id DESC（最新在前）。"""
        with sqlite3.connect(self.db_path) as conn:
            return conn.execute(
                "SELECT ts, status, secs, git_commit FROM timing"
                " WHERE case_name=? ORDER BY id DESC",
                (case_name,),
            ).fetchall()
