# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

project = "TRENTOS"
copyright = "2023-2024, HENSOLDT Cyber GmbH"
author = "HENSOLDT Cyber GmbH"
release = "2023"

html_title = "Documentation"


# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

extensions = [
    "myst_parser",
    "sphinx_copybutton",
    "sphinx_immaterial",
    "sphinxcontrib.mermaid"
]

templates_path = ["_templates"]
exclude_patterns = []


# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

html_theme = "sphinx_immaterial"
html_static_path = ["_static"]

html_logo = "_static/MC_Produkt_Logo_Trentos_gray.png"


html_theme_options = {
    "repo_url": "https://github.com/TRENT-OS/trentos",
    "icon": {
        "repo": "fontawesome/brands/github",
        "edit": "material/file-edit-outline",
    },
    "palette": [
        {
            "media": "(prefers-color-scheme: light)",
            "scheme": "default",
            "primary": "black",
            "accent": "green",
            "toggle": {
                "icon": "material/weather-night",
                "name": "Switch to dark mode",
            },
        },
        {
            "media": "(prefers-color-scheme: dark)",
            "scheme": "slate",
            "primary": "black",
            "accent": "green",
            "toggle": {
                "icon": "material/weather-sunny",
                "name": "Switch to light mode",
            },
        },
    ],
    "features": [
        "navigation.expand",
        "navigation.tabs",
        "navigation.sections",
        "navigation.instant",
        "header.autohide",
        "navigation.top",
        "navigation.tracking",
        "search.highlight",
        "search.share",
        "toc.follow",
        "toc.sticky",
    ],
    "version_dropdown": True
}
