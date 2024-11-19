#!/usr/bin/env python3

import os
from urllib.parse import parse_qs

def main():
    # Fetch QUERY_STRING from the environment
    query_string = os.getenv("QUERY_STRING", "")
    
    # Parse the query string
    params = parse_qs(query_string)
    
    # Generate an HTML response
    print("Content-Type: text/html\n")
    print("<html><head><title>CGI Query Test</title></head>")
    print("<body>")
    print("<h1>CGI Query Test</h1>")
    print("<p><b>Query String:</b> {}</p>".format(query_string))
    print("<p><b>Parsed Parameters:</b></p>")
    print("<ul>")
    for key, values in params.items():
        for value in values:
            print(f"<li>{key}: {value}</li>")
    print("</ul>")
    print("</body></html>")

if __name__ == "__main__":
    main()
