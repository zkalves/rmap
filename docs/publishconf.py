#!/usr/bin/env python
# -*- coding: utf-8 -*- #

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.
#
# Copyright (c) 2026 Ezequiel Alves. All rights reserved.

import os
import sys

sys.path.append(os.path.dirname(__file__))
from pelicanconf import *

# Production settings
SITEURL = 'https://zkalves.github.io/rmap'
RELATIVE_URLS = False

DELETE_OUTPUT_DIRECTORY = True
