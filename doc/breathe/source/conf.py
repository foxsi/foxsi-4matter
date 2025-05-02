import sys
from pathlib import Path
# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

project = 'foxsi-4matter'
copyright = '2024, Thanasi Pantazides, Yixian Zhang, Kris Cooper'
author = 'Thanasi Pantazides, Yixian Zhang, Kris Cooper'
release = 'v1.2.2'

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

extensions = ["sphinx.ext.autodoc", "sphinx.ext.todo", "sphinx.ext.graphviz", "breathe"]
breathe_projects = {"foxsi-4matter": Path(__file__).parent.parent.parent.joinpath('xml/').resolve()}
breathe_default_project = "foxsi-4matter"

todo_include_todos = True

templates_path = ['_templates']
exclude_patterns = [
    "*/env/*",
    "*/doc/*",
    "*/log/*",
    "*/bin/*",
    "*/build/*",
    "foxsi4-commands/*",
    "include/moodycamel/",
    "include/spdlog/*"
]



# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

html_theme = 'furo'
html_static_path = ['_static']
html_logo = "../../assets/Glyph_FOXSI4_text.png"

html_theme_options = {
    "light_css_variables": {
        "font-stack": "Avenir, sans-serif",
        "font-stack--monospace": "PT Mono, monospace",
    },
}