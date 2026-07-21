"""回归测试耗时统计：SQLite 持久化 + 性能回归检测。纯标准库 sqlite3。"""
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
