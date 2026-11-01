"""End-to-end creation, manual save, restart, and error handling."""
import os
from pathlib import Path
import subprocess
import sys
import tempfile


binary = str(Path(sys.argv[1]).resolve())
scratch = Path(sys.argv[2])


def run(directory, commands):
    result = subprocess.run(
        [binary], input=commands, text=True, stdout=subprocess.PIPE,
        stderr=subprocess.PIPE, cwd=directory,
        env={**os.environ, "TERM": "dumb"}, timeout=10,
    )
    assert result.returncode == 0, result.stderr
    return result.stdout


with tempfile.TemporaryDirectory(prefix="rpg-cli-", dir=scratch) as directory:
    output = run(directory, "2\n\n3\n")
    assert "Continue failed:" in output
    # Reject Human, select Orc; reject Warrior, select Knight; save, exit.
    output = run(directory, "1\n1\n2\n8\n1\n1\n2\n2\n1\nCLI Hero\n10\n\n11\n")
    for text in ("Stat bonuses", "Starting stats with Orc", "Signature: Brace", "Game saved"):
        assert text in output, text
    path = Path(directory) / "savegame.dat"
    saved = path.read_bytes()
    # Continue and immediately save: no hidden time, wage, or stat changes.
    output = run(directory, "2\n1\n\n10\n\n11\n")
    assert "Hitpoints: 85/85" in output, "Race bonus missing or applied twice"
    assert b'"CLI Hero"' in saved
    assert "Choose your race:" not in output
    assert saved == path.read_bytes(), "Continue changed the session"
    # Starting a new character and exiting must leave the existing slot alone.
    run(directory, "1\n1\n1\n7\n1\nUnsaved Mage\n11\n")
    assert saved == path.read_bytes(), "Start/Exit overwrote the manual save"
    path.write_text("RPGGAME 999\n", encoding="utf-8")
    output = run(directory, "2\n\n3\n")
    assert "Unsupported save format or version" in output
    path.write_bytes(saved[: len(saved) // 2])
    output = run(directory, "2\n\n3\n")
    assert "Continue failed:" in output

print("Passed CLI save/continue and creation flows")
