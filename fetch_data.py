import argparse
import gzip
import os
import urllib.error
import urllib.request

DATA_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "data")

ITCH_URL = "https://emi.nasdaq.com/ITCH/Nasdaq%20ITCH/itch50_05_15.gz"


def file_size(n: float) -> str:
    for unit in ("B", "KB", "MB", "GB"):
        if n < 1024 or unit == "GB":
            return f"{n:.1f} {unit}" if unit != "B" else f"{int(n)} B"
        n /= 1024.0
    return f"{n:.1f} GB"


def ensure_dir() -> str:
    path = os.path.normpath(DATA_DIR)
    os.makedirs(path, exist_ok=True)
    return path


def download(url: str, dest: str) -> bool:
    print(f"  GET {url}")
    try:
        with urllib.request.urlopen(url, timeout=60) as resp:
            total = int(resp.headers.get("Content-Length") or 0)
            done = 0
            with open(dest, "wb") as out:
                while True:
                    chunk = resp.read(1 << 20)
                    if not chunk:
                        break
                    out.write(chunk)
                    done += len(chunk)
                    print(
                        f"\r  {file_size(done)}"
                        + (f" / {file_size(total)}" if total else ""),
                        end="",
                    )
        print()
        return True
    except urllib.error.HTTPError as e:
        print(f"\n  HTTP {e.code} -- {e.reason}")
        return False
    except Exception as e:
        print(f"\n  failed: {e}")
        return False


# Non-order/trade message types
ITCH_REFERENCE_TYPES = set(b"SRHYLVWKJhO")


def format_timestamp(hhmm: str) -> int:
    h, m = hhmm.split(":")
    return (int(h) * 3600 + int(m) * 60) * 1e9


def itch_window(src, out, start_ns: int, end_ns: int, limit: int) -> dict:
    live: dict = {}
    counts: dict = {}
    written = 0
    in_window = False
    seen = 0

    def emit(body) -> None:
        nonlocal written
        out.write(len(body).to_bytes(2, "big"))
        out.write(body)
        written += 2 + len(body)
        counts[chr(body[0])] = counts.get(chr(body[0]), 0) + 1

    def reduce(ref: int, shares: int) -> None:
        order = live.get(ref)
        if order is None:
            return
        left = int.from_bytes(order[20:24], "big") - shares
        if left <= 0:
            del live[ref]
        else:
            order[20:24] = left.to_bytes(4, "big")

    while written < limit:
        hdr = src.read(2)
        if len(hdr) < 2:
            break
        n = int.from_bytes(hdr, "big")
        body = src.read(n)
        if len(body) < n:
            break
        ts = int.from_bytes(body[5:11], "big")

        if not in_window:
            if ts >= start_ns:
                for order in live.values():
                    emit(order)
                live.clear()
                in_window = True
            else:
                kind = body[0]
                if kind in ITCH_REFERENCE_TYPES:
                    emit(body)
                elif kind in b"AF":
                    live[int.from_bytes(body[11:19], "big")] = bytearray(
                        b"A" + body[1:36]
                    )
                elif kind in b"ECX":
                    reduce(
                        int.from_bytes(body[11:19], "big"),
                        int.from_bytes(body[19:23], "big"),
                    )
                elif kind == ord("D"):
                    live.pop(int.from_bytes(body[11:19], "big"), None)
                elif kind == ord("U"):
                    order = live.pop(int.from_bytes(body[11:19], "big"), None)
                    if order is not None:
                        order[1:11] = body[1:11]
                        order[11:19] = body[19:27]
                        order[20:24] = body[27:31]
                        order[32:36] = body[31:35]
                        live[int.from_bytes(body[19:27], "big")] = order
                seen += 1
                continue

        if ts >= end_ns:
            break
        emit(body)
        if written % (16 * 1024 * 1024) < 64:
            print(f"\r  in window: {file_size(written)} written" + " " * 20, end="")
    print()
    return counts


def cmd_itch(start: str, end: str, max_mb: int) -> int:
    out_dir = ensure_dir()
    dest = os.path.join(
        out_dir, f"itch_{start.replace(':', '')}_{end.replace(':', '')}.bin"
    )
    print(f"  streaming ITCH 5.0, keeping {start}-{end} ET (max {max_mb} MB written)")
    try:
        with (
            urllib.request.urlopen(ITCH_URL, timeout=120) as resp,
            open(dest, "wb") as out,
        ):
            counts = itch_window(
                gzip.GzipFile(fileobj=resp),
                out,
                format_timestamp(start),
                format_timestamp(end),
                max_mb * 1024 * 1024,
            )
    except Exception as e:
        print(f"\n  failed: {e}")
        return 1

    print(f"  wrote {dest} ({file_size(os.path.getsize(dest))})")

    for t, c in sorted(counts.items(), key=lambda kv: -kv[1])[:12]:
        print(f"    {t}  {c:>9,}")

    return 0


def main() -> int:
    ap = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter
    )
    ap.add_argument("--list", action="store_true", help="list sources")
    ap.add_argument("--date", help="UTC date")
    ap.add_argument(
        "--sample",
        action="store_true",
        help="ITCH sample",
    )
    ap.add_argument(
        "--sample-mb",
        type=int,
        default=4096,
        help="ITCH stream of specified size",
    )
    ap.add_argument(
        "--start",
        default="09:30",
        help="window start",
    )
    ap.add_argument(
        "--end",
        default="16:00",
        help="window end",
    )
    args = ap.parse_args()

    if args.source == "itch":
        if not args.sample:
            return 2
        return cmd_itch(args.start, args.end, args.sample_mb)
