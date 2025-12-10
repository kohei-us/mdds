#!/usr/bin/env python

# SPDX-FileCopyrightText: 2025 Kohei Yoshida
#
# SPDX-License-Identifier: MIT

# -*- coding: utf-8 -*-

import sys
import os
import subprocess

rtd_build = os.environ.get('READTHEDOCS', None) == 'True'

if rtd_build:
    subprocess.call("doxygen --version; doxygen doxygen.conf", shell=True)

extensions = ['breathe']

templates_path = ['_templates']

source_suffix = '.rst'
master_doc = 'index'

project = 'mdds'
copyright = '2025, Kohei Yoshida'

version = "3.2"
release = "3.2.0"

exclude_patterns = ['_build']

pygments_style = 'sphinx'

html_theme = 'piccolo_theme'
html_theme_options = {}
html_static_path = ['_static']
htmlhelp_basename = 'mddsdoc'


latex_elements = {}

latex_documents = [
  ('index', 'mdds.tex', 'mdds Documentation',
   'Kohei Yoshida', 'manual'),
]

man_pages = [
    ('index', 'mdds', 'mdds Documentation',
     ['Kohei Yoshida'], 1)
]

texinfo_documents = [
  ('index', 'mdds', 'mdds Documentation',
   'Kohei Yoshida', 'mdds', 'One line description of project.',
   'Miscellaneous'),
]

breathe_projects = {"mdds": "./_doxygen/xml"}
breathe_default_project = "mdds"
breathe_default_members = ('members', 'undoc-members')
