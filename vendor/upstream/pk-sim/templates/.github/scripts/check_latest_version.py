import json
import re
import requests
import sys
import os

try:
    with open("./templates.json", "r") as file:
        templates = json.load(file)
except Exception as e:
    print(f"Error reading templates.json: {str(e)}")
    sys.exit(1)

def parse_github_url(url):
    """Extract owner, repo, and version tag from raw.githubusercontent.com URL"""
    pattern = r"https://raw\.githubusercontent\.com/([^/]+)/([^/]+)/([^/]+)/.+"
    match = re.match(pattern, url)
    if not match:
        return None, None, None
    return match.group(1), match.group(2), match.group(3)

# Use GitHub token for API requests if available
headers = {}
if "GITHUB_TOKEN" in os.environ:
    headers["Authorization"] = f"token {os.environ['GITHUB_TOKEN']}"

has_outdated_templates = False

# Check each template
for template in templates.get("Templates", []):
    template_type = template.get("Type")
    template_name = template.get("Name")
    template_url = template.get("Url")
    
    if not template_url:
        continue
        
    owner, repo, current_version = parse_github_url(template_url)
    
    if not all([owner, repo, current_version]):
        print(f"Warning: Could not parse repository info from {template_url}")
        continue
    
    # Get ALL releases (including pre-releases) using GitHub API
    api_url = f"https://api.github.com/repos/{owner}/{repo}/releases"
    try:
        response = requests.get(api_url, headers=headers)
        response.raise_for_status()
        all_releases = response.json()
        
        # Check if there are any releases
        if not all_releases:
            print(f"No releases found for {owner}/{repo}")
            continue
        
        # Get the most recent release (including pre-releases)
        # Releases are already sorted by date (newest first) by GitHub API
        latest_release = all_releases[0].get("tag_name")
        
        if latest_release and latest_release != current_version:
            print(f"Template for {template_type} {template_name} has later release {latest_release}")
            has_outdated_templates = True
            
    except requests.RequestException as e:
        print(f"Error checking releases for {owner}/{repo}: {str(e)}")

# Exit with error code if any outdated templates found
if has_outdated_templates:
    sys.exit(1)