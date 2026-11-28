# ===-- docs/conf.py ------------------------------------------------------------------===//
#
# midi2 library under the MIT license.
# See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
#
# SPDX-FileCopyrightText: Copyright © 2025 Paul Bowen-Huggett
# SPDX-License-Identifier: MIT
#
# ===----------------------------------------------------------------------------------===//

# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

import os
import pathlib
import subprocess

project = 'midi2'
copyright = '2025-2026, Paul Bowen-Huggett'
author = 'Paul Bowen-Huggett'
version = '0.1'
release = version

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

extensions = [
    'breathe',
    'sphinx.ext.autosectionlabel',
    'sphinx.ext.graphviz',
    'sphinx_design'
]

# Breathe configuration
breathe_projects = { project: './xml/' }
breathe_default_project = project

templates_path = ['_templates']
exclude_patterns = ['_build', 'Thumbs.db', '.DS_Store', 'xml']

# Section labels are referenced as 'document/path:Section title'.
autosectionlabel_prefix_document = True

# The documentation is about C++: make it the default for code and cross-references.
primary_domain = 'cpp'
highlight_language = 'cpp'

# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

html_theme = 'furo'

if os.environ.get('READTHEDOCS', None) == 'True':
    # Run doxygen to produce the XML consumed by Breathe. The overrides are appended to the Doxyfile (later settings
    # win) so that the XML is written to docs/xml and no HTML is produced.
    root = pathlib.Path(__file__).resolve().parent.parent
    overrides = '\n'.join([
        'OUTPUT_DIRECTORY = docs',
        'GENERATE_HTML = NO',
        'GENERATE_XML = YES',
        'XML_OUTPUT = xml',
    ])
    doxyfile = (root / 'Doxyfile').read_text() + '\n' + overrides + '\n'
    subprocess.run(['doxygen', '-'], cwd=root, input=doxyfile, text=True, check=True)
