import os

# 定義路徑
current_dir = os.path.dirname(os.path.abspath(__file__))
web_dir = os.path.join(current_dir, "web")
output_h = os.path.join(current_dir, "include", "web_content.h")

# 讀取原始檔案
with open(os.path.join(web_dir, "index.html"), "r", encoding="utf-8") as f:
    html = f.read()
with open(os.path.join(web_dir, "style.css"), "r", encoding="utf-8") as f:
    css = f.read()
with open(os.path.join(web_dir, "app.js"), "r", encoding="utf-8") as f:
    js = f.read()

# 把 CSS 和 JS 嵌入到 HTML 中對應的位置
# 替換 <link rel="stylesheet" href="style.css">
html = html.replace('<link rel="stylesheet" href="style.css">', f'<style>\n{css}\n</style>')
# 替換 <script src="app.js"></script> (如果有的話，或是直接插在 </body> 前)
if '<script src="app.js"></script>' in html:
    html = html.replace('<script src="app.js"></script>', f'<script>\n{js}\n</script>')
else:
    html = html.replace('</body>', f'<script>\n{js}\n</script>\n</body>')

# 生成 web_content.h
os.makedirs(os.path.dirname(output_h), exist_ok=True)
with open(output_h, "w", encoding="utf-8") as f:
    f.write("#ifndef WEB_CONTENT_H\n#define WEB_CONTENT_H\n\n")
    f.write('const char INDEX_HTML_CONTENT[] = R"html(\n')
    f.write(html)
    f.write('\n)html";\n\n')
    f.write("#endif // WEB_CONTENT_H\n")

print("Web content successfully compiled into web_content.h!")