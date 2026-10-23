#!/bin/bash
set -e

# Install LaTeX dependencies if pdflatex is not available
if ! command -v pdflatex &> /dev/null
then
    echo "pdflatex could not be found."
    if command -v apt-get &> /dev/null; then
        echo "Installing texlive via apt-get..."
        export DEBIAN_FRONTEND=noninteractive
        sudo apt-get update
        sudo apt-get install -y texlive-latex-base texlive-latex-extra texlive-fonts-recommended texlive-science
    elif command -v brew &> /dev/null; then
        echo "Installing mactex via brew..."
        brew install --cask mactex-no-gui
    else
        echo "Error: Please install a LaTeX distribution manually (e.g., MiKTeX for Windows, MacTeX for macOS) and ensure pdflatex is in your PATH."
        exit 1
    fi
else
    echo "pdflatex is already installed."
fi

# Build the whitepaper
echo "Building whitepaper.pdf..."
cd "$(dirname "$0")"
pdflatex -interaction=nonstopmode whitepaper.tex
pdflatex -interaction=nonstopmode whitepaper.tex # Second run for references/TOC

echo "Build complete. Output is paper/whitepaper.pdf"
