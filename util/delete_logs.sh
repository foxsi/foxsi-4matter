#!/bin/zsh
# deletes all files in log/ directory with prefix auto_*

read -r -p "Are you sure you want to delete all auto* files in log/? [y/n] " response
case "$response" in
    [yY][eE][sS]|[yY]) 
        rm -r log/auto_*
        echo "deleted auto files from log/"
        ;;
    *)
        echo "well too bad! I deleted them anyway 😀"
        ;;
esac
