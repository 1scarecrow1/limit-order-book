import re
import sys
from PIL import Image, ImageDraw, ImageFont

FONT = "C:/Windows/Fonts/consola.ttf"
FONT_BOLD = "C:/Windows/Fonts/consolab.ttf"
FONT_SIZE = 16
ROWS = 16
COLS = 72
PAD = 16
TITLE_H = 34
LINE_GAP = 6

BG = (13, 17, 23)
TITLE_BG = (22, 27, 34)
BORDER = (48, 54, 61)
FG = (201, 209, 217)
DIM = (125, 133, 144)
PROMPT = (88, 166, 255)
BID = (63, 185, 80)
ASK = (248, 81, 73)
WHITE = (240, 246, 252)
NEWEST = (28, 35, 45)
DOTS = [(255, 95, 86), (255, 189, 46), (39, 201, 63)]

TOP = re.compile(
    r"^(?P<t>\d\d:\d\d:\d\d)(?P<l1>  bid )(?P<bp>.{11})(?P<x1> x )(?P<bq>\S+\s*)"
    r"(?P<l2>ask )(?P<ap>.{11})(?P<x2> x )(?P<aq>\S+)$"
)
COLORS = {
    "t": DIM,
    "l1": DIM,
    "bp": BID,
    "x1": DIM,
    "bq": BID,
    "l2": DIM,
    "ap": ASK,
    "x2": DIM,
    "aq": ASK,
}

font = ImageFont.truetype(FONT, FONT_SIZE)
bold = ImageFont.truetype(FONT_BOLD, FONT_SIZE)
CW = font.getlength("M")
LH = FONT_SIZE + LINE_GAP
W = int(PAD * 2 + CW * COLS)
H = TITLE_H + PAD * 2 + LH * ROWS


def segments(row):
    """Split one screen row into (text, color, font) runs."""
    kind, text = row
    if kind == "cmd":
        return [("$ ", PROMPT, font), (text, FG, font)]
    if kind == "summary":
        return [(text, WHITE, bold)]
    m = TOP.match(text)
    if not m:
        return [(text, FG, font)]
    return [(m.group(k), COLORS[k], font) for k in COLORS]


def frame(rows, cursor=False, highlight_last=False):
    im = Image.new("RGB", (W, H), BG)
    d = ImageDraw.Draw(im)
    d.rectangle([0, 0, W - 1, TITLE_H], fill=TITLE_BG)
    d.line([0, TITLE_H, W, TITLE_H], fill=BORDER)
    for i, c in enumerate(DOTS):
        cx = PAD + 6 + i * 20
        d.ellipse([cx - 6, TITLE_H // 2 - 6, cx + 6, TITLE_H // 2 + 6], fill=c)
    title = "itch_replay - NASDAQ TotalView-ITCH 5.0"
    d.text(
        ((W - font.getlength(title)) / 2, (TITLE_H - FONT_SIZE) / 2 - 1), title, fill=DIM, font=font
    )
    d.rectangle([0, 0, W - 1, H - 1], outline=BORDER)

    visible = rows[-ROWS:]
    for r, row in enumerate(visible):
        y = TITLE_H + PAD + r * LH
        if highlight_last and r == len(visible) - 1 and row[0] == "top":
            d.rectangle([PAD - 4, y - 2, W - PAD + 4, y + LH - 3], fill=NEWEST)
        x = PAD
        for text, color, f in segments(row):
            d.text((x, y), text, fill=color, font=f)
            x += CW * len(text)
        if cursor and r == len(visible) - 1:
            d.rectangle([x + 1, y, x + CW, y + FONT_SIZE + 1], fill=FG)
    return im


def main():
    src, out, command = sys.argv[1:4]
    raw = open(src, "rb").read()
    encoding = "utf-16" if raw[:2] in (b"\xff\xfe", b"\xfe\xff") else "utf-8-sig"
    lines = [l.rstrip("\r") for l in raw.decode(encoding).split("\n") if l.strip()]

    frames = []
    rows = [("cmd", "")]
    frames.append((frame(rows, cursor=True), 600))
    for i in range(0, len(command), 2):
        rows[-1] = ("cmd", command[: i + 2])
        frames.append((frame(rows, cursor=True), 60))
    frames.append((frame(rows, cursor=True), 400))

    for line in lines:
        kind = "summary" if line.startswith("read ") else "top"
        rows.append((kind, line))
        frames.append((frame(rows, highlight_last=True), 700 if kind == "summary" else 90))
    rows.append(("cmd", ""))
    frames.append((frame(rows, cursor=True), 3500))

    sample = Image.new("RGB", (W, H * 2))
    sample.paste(frames[-3][0], (0, 0))
    sample.paste(frames[-1][0], (0, H))
    palette = sample.quantize(colors=64, method=Image.Quantize.MEDIANCUT)
    images = [im.quantize(palette=palette, dither=Image.Dither.NONE) for im, _ in frames]
    images[0].save(
        out,
        save_all=True,
        append_images=images[1:],
        duration=[d for _, d in frames],
        loop=0,
        optimize=True,
    )
    print(f"wrote {out}: {len(images)} frames, {W}x{H}")


if __name__ == "__main__":
    main()
