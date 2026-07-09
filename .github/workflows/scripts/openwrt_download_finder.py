import sys
from urllib.parse import urljoin
import requests
from bs4 import BeautifulSoup

def url(file, version, target, subtarget):
    base = (
        f"https://downloads.openwrt.org/releases/"
        f"{version}/targets/{target}/{subtarget}/"
    )

    r = requests.get(base, timeout=10)
    r.raise_for_status()

    soup = BeautifulSoup(r.text, "html.parser")

    for a in soup.find_all("a"):
        href = a.get("href", "")
        if href.startswith(f"openwrt-{file}-") and href.endswith(".tar.zst"):
            return urljoin(base, href)

    raise RuntimeError(f"{file} not found")

if __name__ == "__main__":
    print(url(*sys.argv[1:]))
