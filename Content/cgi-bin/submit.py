#!/usr/bin/env python

import cgi
import sys
import os

# Force the output to be in binary mode
sys.stdout = os.fdopen(sys.stdout.fileno(), 'wb')

# Get the form data and set a default value for 'name'
form = cgi.FieldStorage()
name = form.getvalue("name", "Guest")

# HTML content with string formatting using .format()
html = """<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Greeting</title>
    <style>
        body {{
            font-family: Arial, sans-serif;
            text-align: center;
            background-color: #f0f0f0;
            margin: 0;
            padding: 50px;
        }}
        h1 {{
            color: #333;
        }}
        p {{
            color: #555;
        }}
    </style>
</head>
<body>
    <h1>Hello, {}!</h1>
    <p>Welcome to our website. Thank you for submitting your name.</p>
</body>
</html>""".format(name)

# Now that the headers are written, we write the actual HTML content
sys.stdout.write(html.encode('utf-8'))
