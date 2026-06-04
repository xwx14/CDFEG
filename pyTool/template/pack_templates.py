# SPDX-License-Identifier: GPL-3.0
# This file is part of CDFEG.
#
# CDFEG is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# CDFEG is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with CDFEG.  If not, see <https://www.gnu.org/licenses/>.

import sqlite3
import os
import glob

DB_FILE = os.path.join(os.path.dirname(os.path.abspath(__file__)), "templates.db")
TEMPLATE_DIR = os.path.dirname(os.path.abspath(__file__))
TABLE_NAME = "CppTemplates"


def pack():
    if os.path.exists(DB_FILE):
        os.remove(DB_FILE)

    conn = sqlite3.connect(DB_FILE)
    cur = conn.cursor()
    cur.execute(f"CREATE TABLE {TABLE_NAME} (name TEXT PRIMARY KEY, content TEXT)")

    j2_files = sorted(glob.glob(os.path.join(TEMPLATE_DIR, "*.j2")))
    for f in j2_files:
        name = os.path.basename(f)
        with open(f, "r", encoding="utf-8") as fh:
            content = fh.read()
        cur.execute(f"INSERT INTO {TABLE_NAME} (name, content) VALUES (?, ?)", (name, content))
        print(f"  {name}")

    conn.commit()
    conn.close()
    print(f"\n{len(j2_files)} templates packed into {DB_FILE}")


if __name__ == "__main__":
    pack()
