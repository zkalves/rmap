#!/usr/bin/env python
# -*- coding: utf-8 -*- #

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.
#
# Copyright (c) 2026 Ezequiel Alves. All rights reserved.

import os
import re
from pelican.readers import MarkdownReader

AUTHOR = 'rmap Contributors'
SITENAME = 'rmap Documentation Portal'
SITEURL = ''

PATH = '.'
OUTPUT_PATH = '../_site'
CACHE_PATH = '../.pelican-cache'

TIMEZONE = 'UTC'
DEFAULT_LANG = 'en'

# Disable blog/article processing, tags, categories and feed generation for documentation
DIRECT_TEMPLATES = []
PAGINATED_TEMPLATES = {}
INDEX_SAVE_AS = ''
ARTICLE_PATHS = []
PAGE_PATHS = ['.', 'user', 'dev']
IGNORE_FILES = ['pelicanconf.py', 'publishconf.py', '__pycache__', '*.pyc', 'templates']

THEME_TEMPLATES_OVERRIDES = ['templates']

ARCHIVES_SAVE_AS = ''
AUTHORS_SAVE_AS = ''
CATEGORIES_SAVE_AS = ''
TAGS_SAVE_AS = ''
AUTHOR_SAVE_AS = ''
CATEGORY_SAVE_AS = ''
TAG_SAVE_AS = ''
FEED_ALL_ATOM = None
CATEGORY_FEED_ATOM = None
TRANSLATION_FEED_ATOM = None
AUTHOR_FEED_ATOM = None
AUTHOR_FEED_RSS = None

# Preserve nested paths for generated HTML pages
PATH_METADATA = r'(?P<slug_path>.*)\.[a-zA-Z0-9]+'
PAGE_URL = '{slug_path}.html'
PAGE_SAVE_AS = '{slug_path}.html'

# Navigation menu settings
DISPLAY_PAGES_ON_MENU = False
DISPLAY_CATEGORIES_ON_MENU = False
MENUITEMS = (
    ('User Guides', 'user/index.html'),
    ('Developer API', 'dev/index.html'),
)

# Static assets
STATIC_PATHS = []

DEFAULT_PAGINATION = False
RELATIVE_URLS = True

def rewrite_md_links(html):
    def repl(match):
        prefix = match.group(1)
        url = match.group(2)
        quote = match.group(3)
        if url.startswith(('http://', 'https://', '//', 'mailto:', '#')):
            return f'{prefix}{url}{quote}'
        new_url = re.sub(r'^(.*?)(\.md)(#.*)?$', r'\1.html\3', url)
        return f'{prefix}{new_url}{quote}'

    return re.sub(r'(href=[\"\'])([^\"\']+)([\"\'])', repl, html)

# Custom Markdown reader that automatically extracts page title from first # header and rewrites .md links to .html
class AutoTitleMarkdownReader(MarkdownReader):
    def read(self, source_path):
        content, metadata = super().read(source_path)
        if 'title' not in metadata:
            with open(source_path, 'r', encoding='utf-8') as f:
                raw = f.read()
            for line in raw.splitlines():
                m = re.match(r'^\s*#\s+(.+)$', line)
                if m:
                    metadata['title'] = self.process_metadata('title', m.group(1).strip())
                    break
            if 'title' not in metadata:
                base = os.path.splitext(os.path.basename(source_path))[0]
                metadata['title'] = self.process_metadata('title', base)
        if content:
            content = rewrite_md_links(content)
        return content, metadata

READERS = {'md': AutoTitleMarkdownReader}

# Markdown extensions
MARKDOWN = {
    'extension_configs': {
        'markdown.extensions.codehilite': {'css_class': 'highlight'},
        'markdown.extensions.extra': {},
        'markdown.extensions.meta': {},
        'markdown.extensions.toc': {'permalink': True},
        'markdown.extensions.tables': {},
        'markdown.extensions.fenced_code': {},
    },
    'output_format': 'html5',
}
