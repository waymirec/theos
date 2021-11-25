#!/bin/bash
docker run --rm -it -v $(pwd):/root/env -v $(pwd)/../theos-cc:/root/cc theos-buildenv
