import os
import re
import base64
import requests
import mimetypes
from bs4 import BeautifulSoup
from urllib.parse import urljoin, urlparse

# Paths
INPUT_HTML = "Biochar_Toilet_Slide_Deck.html"
OUTPUT_HTML = "Biochar_Toilet_Slide_Deck_Offline.html"

def get_base64_from_url(url):
    try:
        headers = {
            'User-Agent': 'BiocharToiletOfflineExporter/1.0 (https://github.com/PubInv/Biochar-Toilet; your@email.com)'
        }
        response = requests.get(url, headers=headers, timeout=10)
        response.raise_for_status()
        mime_type = response.headers.get('content-type')
        if not mime_type:
            mime_type = mimetypes.guess_type(url)[0] or 'application/octet-stream'
        b64_data = base64.b64encode(response.content).decode('utf-8')
        return f"data:{mime_type};base64,{b64_data}"
    except Exception as e:
        print(f"Error fetching URL {url}: {e}")
        # Return a transparent 1x1 pixel image if it's an image
        if url.endswith('.jpg') or url.endswith('.png') or url.endswith('.jpeg'):
            return "data:image/gif;base64,R0lGODlhAQABAIAAAAAAAP///yH5BAEAAAAALAAAAAABAAEAAAIBRAA7"
        return url

from urllib.parse import unquote

def get_base64_from_file(filepath):
    try:
        # Remove query params if any
        if '?' in filepath:
            filepath = filepath.split('?')[0]
        # Decode urlencoded path
        filepath = unquote(filepath)

        if not os.path.exists(filepath):
            print(f"File not found: {filepath}")
            return filepath
        mime_type = mimetypes.guess_type(filepath)[0] or 'application/octet-stream'
        with open(filepath, 'rb') as f:
            b64_data = base64.b64encode(f.read()).decode('utf-8')
        return f"data:{mime_type};base64,{b64_data}"
    except Exception as e:
        print(f"Error reading file {filepath}: {e}")
        return filepath

def is_url(path):
    return path.startswith('http://') or path.startswith('https://')

def replace_css_urls(css_text, base_url=""):
    # Finds url(...) in css
    url_pattern = re.compile(r'url\((["\']?)(.*?)\1\)')
    def replace_url(match):
        quote = match.group(1)
        url = match.group(2)
        if url.startswith('data:'):
            return match.group(0)
        if is_url(url):
            new_url = get_base64_from_url(url)
        else:
            if base_url:
                full_url = urljoin(base_url, url)
                new_url = get_base64_from_url(full_url)
            else:
                new_url = get_base64_from_file(url)
        return f'url({quote}{new_url}{quote})'

    return url_pattern.sub(replace_url, css_text)

def main():
    if not os.path.exists(INPUT_HTML):
        print(f"Error: {INPUT_HTML} not found.")
        return

    with open(INPUT_HTML, 'r', encoding='utf-8') as f:
        html_content = f.read()

    soup = BeautifulSoup(html_content, 'html.parser')

    # 1. Process inline styles for background images
    for tag in soup.find_all(style=True):
        style = tag['style']
        if 'url(' in style:
            tag['style'] = replace_css_urls(style)

    # 2. Process external stylesheets
    for link in soup.find_all('link', rel='stylesheet'):
        href = link.get('href')
        if not href:
            continue
        print(f"Processing CSS: {href}")
        if is_url(href):
            try:
                response = requests.get(href, timeout=10)
                response.raise_for_status()
                css_text = response.text

                # Replace URLs inside the CSS
                css_text = replace_css_urls(css_text, base_url=href)

                # Create a new style tag
                style_tag = soup.new_tag('style')
                style_tag.string = css_text
                link.replace_with(style_tag)
            except Exception as e:
                print(f"Error processing CSS {href}: {e}")
        else:
            try:
                with open(href, 'r', encoding='utf-8') as f:
                    css_text = f.read()
                css_text = replace_css_urls(css_text)
                style_tag = soup.new_tag('style')
                style_tag.string = css_text
                link.replace_with(style_tag)
            except Exception as e:
                print(f"Error processing local CSS {href}: {e}")

    # 3. Process image tags
    for img in soup.find_all('img'):
        src = img.get('src')
        if not src or src.startswith('data:'):
            continue
        print(f"Processing image: {src}")
        if is_url(src):
            img['src'] = get_base64_from_url(src)
        else:
            # src is usually relative to Presentations/
            # For things like ../images/xyz.webp
            img['src'] = get_base64_from_file(src)

    # 4. Process video sources
    for source in soup.find_all('source'):
        src = source.get('src')
        if not src or src.startswith('data:'):
            continue
        print(f"Processing video: {src}")
        if is_url(src):
            source['src'] = get_base64_from_url(src)
        else:
            source['src'] = get_base64_from_file(src)

    # 5. Process external javascript
    for script in soup.find_all('script'):
        src = script.get('src')
        if not src:
            continue
        print(f"Processing script: {src}")
        try:
            if is_url(src):
                response = requests.get(src, timeout=10)
                response.raise_for_status()
                script_text = response.text
            else:
                filepath = src
                if '?' in filepath:
                    filepath = filepath.split('?')[0]
                filepath = unquote(filepath)
                with open(filepath, 'r', encoding='utf-8') as f:
                    script_text = f.read()

            new_script = soup.new_tag('script')
            new_script.string = script_text
            script.replace_with(new_script)
        except Exception as e:
            print(f"Error processing script {src}: {e}")

    # Save the standalone HTML
    with open(OUTPUT_HTML, 'w', encoding='utf-8') as f:
        f.write(str(soup))

    print(f"Successfully generated {OUTPUT_HTML} (size: {os.path.getsize(OUTPUT_HTML)} bytes)")

if __name__ == '__main__':
    main()
