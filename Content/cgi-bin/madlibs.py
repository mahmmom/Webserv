#!/usr/bin/env python3

import os
import cgi
from urllib.parse import parse_qs

def main():
    # Retrieve GET parameters
    query_string = os.environ.get("QUERY_STRING", "")
    params = parse_qs(query_string)

    # Extract user inputs (with default values if not provided)
    name = params.get("name", ["Someone"])[0]
    place = params.get("place", ["somewhere"])[0]
    object_ = params.get("object", ["something"])[0]
    action = params.get("action", ["do something"])[0]

    # Generate the Mad Libs story
    story = f"""
    <p>Once upon a time, <strong>{name}</strong> went to <strong>{place}</strong>. 
    There, they found a magical <strong>{object_}</strong> that could <strong>{action}</strong>! 
    It was a day to remember.</p>
    """

    # Output the HTML
    # print("Content-Type: text/html\n")
    print(f"""
    <!DOCTYPE html>
    <html lang="en">
    <head>
        <meta charset="UTF-8">
        <title>Mad Libs Generator</title>
        <style>
            body {{
                font-family: Arial, sans-serif;
                background-color: #f0f8ff;
                padding: 20px;
                text-align: center;
            }}
            .story {{
                font-size: 1.2em;
                color: #333;
            }}
        </style>
    </head>
    <body>
        <h1>Your Mad Libs Story</h1>
        <div class="story">
            {story}
        </div>
        <a href="../index/index.html">Go Back</a>
    </body>
    </html>
    """)

if __name__ == "__main__":
    main()
