import argparse
import json
import os
import re
import subprocess
import requests
from packaging import version as semver

GITHUB_API = "https://api.github.com"
IGNORED_CONTRIBUTORS = ['abi-git-user', 'github-actions[bot]']

HEADERS = {
    "Accept": "application/vnd.github+json",
    "Authorization": f"Bearer {os.environ['GH_TOKEN']}"
}

TAG_PATTERN = re.compile(r"^v(\d+\.\d+\.\d+)$")
LABEL_PRIORITY = []


class Contributor:

    def __init__(self, avatar_url, user_url):
        self._avatar_url = avatar_url
        self._user_url = user_url

    @property
    def avatar_url(self):
        return self._avatar_url

    @property
    def user_url(self):
        return self._user_url

    def __eq__(self, other):
        return isinstance(other, Contributor) and \
            self.user_url == other.user_url and \
            self.avatar_url == other.avatar_url

    def __hash__(self):
        return hash((self.user_url, self.avatar_url))


def run(cmd):
    return subprocess.check_output(cmd, text=True).strip()


def find_previous_source_tag(end_tag):
    tags = run(["git", "tag"]).splitlines()
    print('Found tags:', tags)
    valid = []

    m = None if end_tag == "HEAD" else TAG_PATTERN.match(end_tag)
    end_version = semver.parse(m.group(1)) if m else None

    print(f"Finding previous version tag before {end_tag} (version: {end_version})")
    for t in tags:
        m = TAG_PATTERN.match(t)
        if not m:
            continue

        v = semver.parse(m.group(1))
        if end_version and v >= end_version:
            continue

        valid.append((v, t))

    print(valid)
    if not valid:
        raise RuntimeError("No valid vX.Y.Z tags found")

    valid.sort(reverse=True)
    previous_source_tag = f"source-{valid[0][1]}"
    if not previous_source_tag in tags:
        raise RuntimeError(f"Previous source tag {previous_source_tag} for {valid[0][1]} not found in git tags")

    print(f"Previous source tag found: {previous_source_tag} with version {valid[0][0]}")
    return previous_source_tag


def get_merge_commits(start, end="HEAD"):
    log = run([
        "git", "log",
        f"{start}..{end}",
        "--merges",
        "--first-parent",
        "--pretty=%h"
    ])
    print(f"Git log between {start} and {end}:\n{log}")
    return log.splitlines()


def fetch_associated_pr_number(commit):
    url = f"{GITHUB_API}/repos/cellml/libcellml/commits/{commit}/pulls"
    r = requests.get(url, headers=HEADERS)
    r.raise_for_status()
    prs = r.json()
    if not prs:
        print(f"No associated PR found for commit {commit}.")
        return None
    pr_number = prs[0]["number"]
    print(f"Associated PR #{pr_number} found for commit {commit}.")
    return pr_number


def fetch_pr(org_repo, pr_number):
    url = f"{GITHUB_API}/repos/{org_repo}/pulls/{pr_number}"
    r = requests.get(url, headers=HEADERS)
    r.raise_for_status()
    return r.json()


def choose_primary_label(labels):
    names = [l["name"] for l in labels]
    for p in LABEL_PRIORITY:
        if p in names:
            return p
    return names[0] if names else "No category"


def extract_summary(pr):
    primary_label = choose_primary_label(pr["labels"])
    secondary_labels = [l["name"] for l in pr["labels"] if l["name"] != primary_label]
    return {
        "title": pr["title"],
        "label": primary_label,
        "secondary_labels": secondary_labels,
        "number": pr["number"],
        "url": pr["html_url"],
        "user": pr["user"]["login"],
        "user_url": pr["user"]["html_url"],
        "avatar_url": pr["user"]["avatar_url"],
    }


def write_out_to_changelog_file(sorted_summaries, tag_end):
    current_label = ''
    file_name = f'changelog_{tag_end}.rst'
    with open(file_name, 'w') as f:

        changelog_title = f'libCellML {tag_end} Changelog'
        f.write(f'{changelog_title}\n')
        f.write('=' * len(changelog_title))
        f.write('\n')

        contributors = []
        for summary in sorted_summaries:
            if current_label != summary['label']:
                current_label = summary['label']
                f.write(f'\n{current_label}\n')
                f.write('-' * len(current_label))
                f.write('\n\n')

            title = summary['title'][:-1] if summary['title'].endswith('.') else summary['title']
            user_link = f'`@{summary["user"]} <{summary["user_url"]}>`_'
            pr_link = f'`#{summary["number"]} <{summary["url"]}>`_'
            suffix = ""
            if summary.get("secondary_labels"):
                suffix = f" (also: {', '.join(summary['secondary_labels'])})"
            f.write(f'* {title}{suffix} by {user_link} [{pr_link}].\n')
            contributors.append(Contributor(summary['avatar_url'], summary['user_url']))

        contributors = list(set(contributors))
        if contributors:
            section_title = 'Contributors'
            f.write(f'\n{section_title}\n')
            f.write('-' * len(section_title))
            f.write('\n\n')
        for contributor in contributors:
            f.write(f'.. image:: {contributor.avatar_url}\n   :target: {contributor.user_url}'
                    f'\n   :height: 32\n   :width: 32\n')

    print(f'Changelog written to: {file_name}.')


def process_arguments():
    parser = argparse.ArgumentParser(description="Create a simple change log from merged pull requests from a GitHub "
                                                 "project.")
    parser.add_argument("-p", "--project",
                        help="GitHub project to work with, default 'cellml/libcellml'.", default="cellml/libcellml")
    # parser.add_argument("-r", "--local-repo",
    #                     help="The location of the project repository. Absolute path or relative path relative to the "
    #                          "current working directory.", default=None)
    parser.add_argument("-t", "--tag-end-display-name",
                        help="Override the tag end label display name.", default=None)
    parser.add_argument("tag_start", nargs='?', default="PREV",)
    parser.add_argument("tag_end", nargs='?', default="HEAD")

    return parser.parse_args()


if __name__ == "__main__":
    args = process_arguments()
    previous_source_tag = find_previous_source_tag(args.tag_end) if args.tag_start == "PREV" else args.tag_start
    commits = get_merge_commits(previous_source_tag, 'source-' + args.tag_end if args.tag_end != "HEAD" else "HEAD")
    print(f"Found {len(commits)} merge commits between {previous_source_tag} and {args.tag_end}.")

    pr_numbers = []
    for commit in commits:
        pr_number = fetch_associated_pr_number(commit)
        if pr_number is not None:
            pr_numbers.append(pr_number)
    print(f"Extracted {len(pr_numbers)} PR numbers from merge commits.")

    summaries = []
    for pr_number in pr_numbers:
        pr = fetch_pr(args.project, pr_number)

        if pr["user"]["login"] in IGNORED_CONTRIBUTORS:
            continue

        if pr["merged"]:
            summaries.append(extract_summary(pr))

    sorted_summaries = sorted(summaries, key=lambda x: x["label"])
    print(f"Extracted summaries for {len(sorted_summaries)} merged PRs.")
    print(sorted_summaries)

    tag_end_label = "latest" if args.tag_end == "HEAD" else f"{args.tag_end}"
    tag_end_label = tag_end_label if args.tag_end_display_name is None else args.tag_end_display_name
    write_out_to_changelog_file(sorted_summaries, tag_end=tag_end_label)
