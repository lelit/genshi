#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Copyright (C) 2006-2010 Edgewall Software
# All rights reserved.
#
# This software is licensed as described in the file COPYING, which
# you should have received as part of this distribution. The terms
# are also available at http://genshi.edgewall.org/wiki/License.
#
# This software consists of voluntary contributions made by many
# individuals. For the exact contribution history, see the revision
# history and logs, available at http://genshi.edgewall.org/log/.

import os
try:
    from setuptools import setup, Extension
except ImportError:
    from distutils.core import setup, Extension
import sys

sys.path.append(os.path.join('doc', 'common'))
try:
    from doctools import build_doc, test_doc
except ImportError:
    build_doc = test_doc = None


cmdclass = {'build_doc': build_doc, 'test_doc': test_doc}

setup(
    name = 'Genshi',
    version = '0.8',
    description = 'A toolkit for generation of output for the web',
    long_description = \
"""Genshi is a Python library that provides an integrated set of
components for parsing, generating, and processing HTML, XML or
other textual content for output generation on the web. The major
feature is a template language, which is heavily inspired by Kid.""",
    author = 'Edgewall Software',
    author_email = 'info@edgewall.org',
    license = 'BSD',
    url = 'http://genshi.edgewall.org/',
    download_url = 'http://genshi.edgewall.org/wiki/Download',

    classifiers = [
        'Development Status :: 4 - Beta',
        'Environment :: Web Environment',
        'Intended Audience :: Developers',
        'License :: OSI Approved :: BSD License',
        'Operating System :: OS Independent',
        'Programming Language :: Python',
        'Programming Language :: Python :: 3',
        'Programming Language :: Python :: 3.5',
        'Programming Language :: Python :: 3.6',
        'Topic :: Internet :: WWW/HTTP :: Dynamic Content',
        'Topic :: Software Development :: Libraries :: Python Modules',
        'Topic :: Text Processing :: Markup :: HTML',
        'Topic :: Text Processing :: Markup :: XML'
    ],
    keywords = ['python.templating.engines'],

    packages = [
        'genshi', 'genshi.filters', 'genshi.template',
        'genshi.tests', 'genshi.filters.tests',
        'genshi.template.tests',
        'genshi.template.tests.templates',
    ],

    test_suite = 'genshi.tests.suite',

    extras_require = {
        'i18n': ['Babel>=0.8'],
        'plugin': ['setuptools>=0.6a2']
    },
    entry_points = """
    [babel.extractors]
    genshi = genshi.filters.i18n:extract[i18n]

    [python.templating.engines]
    genshi = genshi.template.plugin:MarkupTemplateEnginePlugin[plugin]
    genshi-markup = genshi.template.plugin:MarkupTemplateEnginePlugin[plugin]
    genshi-text = genshi.template.plugin:TextTemplateEnginePlugin[plugin]
    """,

    ext_modules = [
        Extension('genshi._speedups', ['genshi/_speedups.c']),
    ],

    cmdclass = cmdclass,

    include_package_data = True,
)
