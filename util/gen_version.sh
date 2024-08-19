#!/bin/bash

if [ "$(uname)" == "Darwin" ]; then
   script_name=$(greadlink -f "$0")
else
   script_name=$(readlink -f "$0")
fi
script_dir=$(dirname "$script_name")
      
include_file="$script_dir"/../include/GRSIDataVersion.h

release_commit=$(git describe --abbrev=0 --match="v*" --tags)
release_num=${release_commit//v/}
release_time=$(git show -s --pretty=format:%ai "$release_commit" | tail -n 1)
release_name=$(git rev-parse "$release_commit" | xargs git cat-file -p | tail -n1)

git_commit=$(git describe --tags)
git_branch=$(git branch | sed -n '/\* /s///p')
git_commit_time=$(git show -s --pretty=format:%ai "$git_commit" | tail -n 1)

read -r -d '' file_contents <<EOF
#ifndef GRSIDATAVERSION_H
#define GRSIDATAVERSION_H

#define GRSIDATA_RELEASE "$release_num"
#define GRSIDATA_RELEASE_TIME "$release_time"
#define GRSIDATA_RELEASE_NAME "$release_name"

#define GRSIDATA_GIT_COMMIT "$git_commit"
#define GRSIDATA_GIT_BRANCH "$git_branch"
#define GRSIDATA_GIT_COMMIT_TIME "$git_commit_time"

#endif /* GRSIDATAVERSION_H */
EOF

echo "$file_contents" > "$include_file"
