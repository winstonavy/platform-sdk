#!/bin/sh

set -e

yum update
yum --yes build-dep ./
dpkg-buildpackage
