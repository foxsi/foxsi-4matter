import sys
# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information
sys.path.append("/Users/thanasi/Documents/FOXSI/Data/formatter/foxsi-4matter/doc/breathe")

project = 'foxsi-4matter'
copyright = '2024, Thanasi Pantazides, Yixian Zhang, Kris Cooper'
author = 'Thanasi Pantazides, Yixian Zhang, Kris Cooper'
release = 'v1.0.1'

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

extensions = ["sphinx.ext.autodoc", "sphinx.ext.todo", "sphinx.ext.graphviz", "breathe"]
breathe_projects = {"foxsi-4matter": "/Users/thanasi/Documents/FOXSI/Data/formatter/foxsi-4matter/doc/xml"}
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
html_logo = "../../assets/FOXSI4_text.png"

html_theme_options = {
    "light_css_variables": {
        "font-stack": "Avenir, sans-serif",
        "font-stack--monospace": "PT Mono, monospace",
    },
}