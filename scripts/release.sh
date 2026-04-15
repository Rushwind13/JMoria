#!/usr/bin/env bash
# scripts/release.sh — JMoria release helper
# Usage: ./scripts/release.sh <version>
# Example: ./scripts/release.sh 0.61
#
# Steps this script handles:
#   1. Validate preconditions (clean tree, on develop, up to date)
#   2. Create release/<version> branch from develop
#   3. Bump VERSION in src/Constants.h
#   4. Build and test
#   5. Commit, push, and print next steps
#
# What you do after this script:
#   - Create PR from release/<version> → main on GitHub
#   - Squash-merge the PR
#   - Create a Release on GitHub (which creates the tag)
#   - Run: ./scripts/release.sh --finish <version>

set -euo pipefail

REPO_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$REPO_ROOT"

CONSTANTS_FILE="src/Constants.h"

usage() {
    echo "Usage: $0 <version>"
    echo "       $0 --finish <version>"
    echo ""
    echo "Examples:"
    echo "  $0 0.61          # Create release branch, bump version, build, test"
    echo "  $0 --finish 0.61 # After PR merged: pull main, merge back to develop"
    exit 1
}

# --- Finish mode: merge main back to develop after PR is merged ---
finish_release() {
    local VERSION="$1"
    echo "=== Finishing release $VERSION ==="

    echo "--- Fetching latest ---"
    git fetch origin

    echo "--- Updating local main ---"
    git checkout main
    git pull origin main

    # Verify the tag exists (created via GitHub Release UI)
    if ! git tag -l "$VERSION" | grep -q "$VERSION"; then
        echo "WARNING: Tag '$VERSION' not found. Did you create the GitHub Release?"
        echo "You can continue, but remember to create it."
    else
        echo "--- Tag $VERSION found ---"
    fi

    echo "--- Merging main back to develop ---"
    git checkout develop
    git pull origin develop
    git merge main -m "Merge main back to develop after release $VERSION"
    git push origin develop

    echo "--- Cleaning up release branch ---"
    git branch -d "release/$VERSION" 2>/dev/null || true
    git push origin --delete "release/$VERSION" 2>/dev/null || true

    echo ""
    echo "=== Release $VERSION finished! ==="
    echo "develop is up to date with main."
}

# --- Main release flow ---
if [[ $# -lt 1 ]]; then
    usage
fi

if [[ "$1" == "--finish" ]]; then
    [[ $# -lt 2 ]] && usage
    finish_release "$2"
    exit 0
fi

VERSION="$1"
BRANCH="release/$VERSION"

echo "=== Starting release $VERSION ==="

# 1. Precondition checks
echo "--- Checking preconditions ---"

CURRENT_BRANCH=$(git branch --show-current)
if [[ "$CURRENT_BRANCH" != "develop" ]]; then
    echo "ERROR: Must be on 'develop' branch (currently on '$CURRENT_BRANCH')"
    exit 1
fi

if ! git diff --quiet || ! git diff --cached --quiet; then
    echo "ERROR: Working tree is not clean. Commit or stash changes first."
    exit 1
fi

git fetch origin
LOCAL=$(git rev-parse develop)
REMOTE=$(git rev-parse origin/develop)
if [[ "$LOCAL" != "$REMOTE" ]]; then
    echo "ERROR: Local develop is not up to date with origin/develop."
    echo "  Local:  $LOCAL"
    echo "  Remote: $REMOTE"
    echo "Run 'git pull' first."
    exit 1
fi

OLD_VERSION=$(grep '#define VERSION' "$CONSTANTS_FILE" | sed 's/.*"\(.*\)".*/\1/')
echo "  Current VERSION: $OLD_VERSION"
echo "  New VERSION:     $VERSION"

# 2. Create release branch
echo "--- Creating branch $BRANCH ---"
git checkout -b "$BRANCH"

# 3. Bump version
echo "--- Bumping VERSION to $VERSION ---"
sed -i '' "s/#define VERSION \"$OLD_VERSION\"/#define VERSION \"$VERSION\"/" "$CONSTANTS_FILE"
grep '#define VERSION' "$CONSTANTS_FILE"

# 4. Build and test
echo "--- Building (ascii + test) ---"
make clean
make ascii test

echo "--- Running tests ---"
cd test
if ./runtests.sh; then
    echo "--- All tests passed ---"
else
    echo "ERROR: Tests failed!"
    echo "Fix the issues, then manually:"
    echo "  git add -A && git commit && git push -u origin $BRANCH"
    cd "$REPO_ROOT"
    exit 1
fi
cd "$REPO_ROOT"

# 5. Commit and push
echo "--- Committing version bump ---"
git add -A
git commit -m "Release $VERSION: bump version"

echo "--- Pushing $BRANCH ---"
git push -u origin "$BRANCH"

echo ""
echo "============================================"
echo "  Release branch $BRANCH pushed!"
echo "============================================"
echo ""
echo "Next steps:"
echo "  1. Create PR: $BRANCH → main"
echo "     https://github.com/Rushwind13/JMoria/compare/main...$BRANCH"
echo "  2. Squash-merge the PR on GitHub"
echo "  3. Create a GitHub Release at:"
echo "     https://github.com/Rushwind13/JMoria/releases/new"
echo "     - Tag: $VERSION  (create new tag)"
echo "     - Target: main"
echo "     - Title: <your release title>"
echo "     - Paste release notes from doc/RELEASE-${VERSION/0./0.}.md"
echo "  4. Run: ./scripts/release.sh --finish $VERSION"
