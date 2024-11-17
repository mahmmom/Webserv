#!/usr/bin/env python3
import os
import sys
import re
import cgitb
cgitb.enable()

def parse_headers(raw_headers):
    headers = {}
    for line in raw_headers.split('\r\n'):
        if ': ' in line:
            key, value = line.split(': ', 1)
            headers[key.lower()] = value.strip()
    return headers

def get_content_type(filepath):
    _, ext = os.path.splitext(filepath)
    if ext.lower() in ['.jpg', '.jpeg', '.png', '.gif']:
        return 'image'
    elif ext.lower() in ['.mp4', '.webm']:
        return 'video'
    return None

def generate_response(filepath, filename):
    return f"""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Upload Result</title>
    <style>
        body {{
            font-family: Arial, sans-serif;
            max-width: 800px;
            margin: 0 auto;
            padding: 20px;
            text-align: center;
        }}
        .content {{
            background-color: #f5f5f5;
            border-radius: 8px;
            padding: 20px;
            margin-top: 20px;
        }}
        img, video {{
            max-width: 100%;
            height: auto;
            border-radius: 8px;
            margin: 20px 0;
        }}
        .button {{
            display: inline-block;
            padding: 10px 20px;
            background-color: #4CAF50;
            color: white;
            text-decoration: none;
            border-radius: 4px;
            margin: 10px;
        }}
        .button:hover {{
            background-color: #45a049;
        }}
    </style>
</head>
<body>
    <h1>Upload Result</h1>
    <div class="content">
    {generate_content(filepath, filename)}
    </div>
</body>
</html>"""

def generate_content(filepath, filename):
    file_type = get_content_type(filepath)
    
    if file_type == 'image':
        content = f"""
        <h2>Image '{filename}' uploaded successfully</h2>
        <img src="../uploads/{filename}" alt="{filename}">"""
    elif file_type == 'video':
        content = f"""
        <h2>Video '{filename}' uploaded successfully</h2>
        <video controls>
            <source src="../uploads/{filename}" type="video/mp4">
            Your browser does not support the video tag.
        </video>"""
    else:
        content = f"""
        <h2>File '{filename}' uploaded successfully</h2>
        <p>File name: {filename}</p>"""

    content += f"""
        <div>
            <a href="../uploads/{filename}" class="button">View File</a>
            <a href="../index/index.html" class="button">Back to Home</a>
        </div>"""
    return content

def parse_multipart_form_data(stdin, boundary, buffer_size=1000000):
    accumulated_chunks = bytearray()
    while True:
        chunk = stdin.buffer.read(buffer_size)
        accumulated_chunks.extend(chunk)
        if b'\r\n\r\n' in accumulated_chunks:
            break

    headers_part, rest_of_data = accumulated_chunks.split(b'\r\n\r\n', 1)
    raw_headers = headers_part.decode('utf-8')
    headers = parse_headers(raw_headers)

    content_disposition = headers.get('content-disposition', '')
    filename = re.findall('filename="([^"]+)"', content_disposition)
    if not filename:
        return "<h2>Filename not found in headers.</h2>"
    filename = filename[0]

    file_start = rest_of_data.find(boundary.encode()) - 4
    file_data = rest_of_data[:file_start] if file_start > 0 else rest_of_data

    filepath = f"Content/uploads/{filename}"
    with open(filepath, 'wb') as f:
        f.write(file_data)
        while True:
            chunk = stdin.buffer.read(buffer_size)
            if not chunk:
                break
            end_of_file_data = chunk.find(boundary.encode())
            if end_of_file_data >= 0:
                f.write(chunk[:end_of_file_data - 4])
                break
            f.write(chunk)

    return generate_response(filepath, filename)

def main():
    content_type = os.environ.get('CONTENT_TYPE', '')
    boundary = content_type.split('boundary=')[-1]
    if not boundary:
        sys.stdout.buffer.write(b"Content-Type: text/plain\n\n")
        sys.stdout.buffer.write(b"No boundary found in Content-Type header.")
        return

    # sys.stdout.buffer.write(b"Content-Type: text/plain\n\n")
    sys.stdout.buffer.write(parse_multipart_form_data(sys.stdin, boundary).encode())

if __name__ == "__main__":
    main()