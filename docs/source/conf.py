# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html
import datetime
from pathlib import Path

import routingblocks

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

project = 'RoutingBlocks'
authors = 'Patrick Sean Klein'
copyright = f'2023 - {datetime.date.today().year}, {authors}'
release = '01.04.2023'

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

routingblocks_import_path = Path(routingblocks.__file__).parent

extensions = [
    'autoapi.extension',
    'sphinx.ext.autodoc',
    'sphinx.ext.autodoc.typehints',
    'sphinx.ext.autosummary',
    'sphinx.ext.duration',
    'sphinx.ext.doctest'
]

autosummary_generate = True
autoapi_type = "python"
autoapi_dirs = [str(routingblocks_import_path)]
autoapi_options = ["undoc-members", "members", "special-members"]

autoapi_generate_api_docs = False
autoapi_add_toctree_entry = False
autoapi_add_objects_to_toctree = False

autoapi_python_class_content = "class"
autoapi_member_order = "bysource"

autodoc_typehints = "description"

templates_path = ['_templates']
exclude_patterns = []

# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

html_theme = 'sphinx_rtd_theme'
html_static_path = ['_static']
