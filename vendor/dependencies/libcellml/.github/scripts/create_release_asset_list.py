"""
This script creates a list of release assets from a specified release defined by
tag name.
"""
import argparse
import json
import os
import requests


class CommunicationError(Exception):
    pass


class ReleaseNotFound(Exception):
    pass


class ReleaseAssetCountError(Exception):
    pass


FETCH_X_RELEASE_ASSETS = 20


def _ask_github_for_release_assets(project_org, project_name, release_tag):
    libcellml_authorised_personal_access_token = os.environ.get("LIBCELLML_AUTHORISED_PERSONAL_ACCESS_TOKEN", "XXX")

    query = f"""query {{
      repository(owner:"{project_org}", name:"{project_name}") {{
        release(tagName:"{release_tag}") {{
          name,
          releaseAssets(first:{FETCH_X_RELEASE_ASSETS}) {{
            totalCount,
            edges {{
              node {{
                contentType,
                downloadUrl,
                name
              }}
            }}
          }}
        }}
      }}
    }}"""
    url = "https://api.github.com/graphql"
    r = requests.post(url, json={'query': query}, headers={
        'Accept': 'application/vnd.github.v3+json',
        'Authorization': f'token {libcellml_authorised_personal_access_token}'})

    if r.status_code != 200:
        raise CommunicationError(f'Got status code {r.status_code} from Github.')

    return json.loads(r.text)


def _check_release_asset_count(release_data):
    total_count = release_data["release"]["releaseAssets"]["totalCount"]
    if total_count > FETCH_X_RELEASE_ASSETS:
        raise ReleaseAssetCountError("The release has more assets than the number of assets requested.")


def process_arguments():
    parser = argparse.ArgumentParser(description="List release assets from a GitHub release.")
    parser.add_argument("-p", "--project",
                        help="GitHub project to work with, default 'cellml/libcellml'.", default="cellml/libcellml")
    parser.add_argument("release_tag")

    return parser


def main():
    parser = process_arguments()
    args = parser.parse_args()

    project = args.project
    release_tag = args.release_tag

    project_org, project_repo = project.split('/')
    response_data = _ask_github_for_release_assets(project_org, project_repo, release_tag)

    release_data = response_data["data"]["repository"]
    if release_data["release"] is None:
        raise ReleaseNotFound(f"Release '{release_tag}' not found.")

    _check_release_asset_count(release_data)

    with open("release_assets.json", "w") as f:
        json.dump(release_data, f)


if __name__ == "__main__":
    main()
