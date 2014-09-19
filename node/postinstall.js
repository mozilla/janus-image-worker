'use strict';

var http = require('http');
var https = require('https');
var exec = require('child_process').exec;
var chalk = require('chalk');
var tar = require('tar');
var zlib = require('zlib');
var fs = require('fs-extra');

function exitOnError(message, err) {
  console.log(chalk.red('✗ ' + message));
  console.log(err);
  process.exit(1);
}


var mozjpegUntar = tar.Extract({ path: 'src/mozjpeg', strip: 1 });
var jpegturboUntar = tar.Extract({ path: 'src/jpeg-turbo', strip: 1 });

// set to true when the first binary has been built (use to check wether we can
// build the recompressor or not).
var firstBinBuilt = false;

mozjpegUntar.on('end', buildMozjpeg);
jpegturboUntar.on('end', buildJpegturbo);

var jpwgturboReq =
  http.get('http://iweb.dl.sourceforge.net/project/libjpeg-turbo/1.3.1/libjpeg-turbo-1.3.1.tar.gz',
  function(res) {
    res.pipe(zlib.createGunzip()).pipe(jpegturboUntar);
  });

var mozjpegReq =
  https.get('https://codeload.github.com/mozilla/mozjpeg/tar.gz/v2.1',
  function(res) {
    res.pipe(zlib.createGunzip()).pipe(mozjpegUntar);
  });


function buildMozjpeg() {
  console.log(chalk.blue('✓ mozjpeg downloaded successfully'));
  exec('cd src/mozjpeg && autoreconf -fiv && ./configure && make',
       function(err, stdout, stderr) {
         if (err) {
           exitOnError('Unable to build mozjpeg. ', stderr);
         }

         console.log(chalk.blue('✓ mozjpeg built successfully.'));

         if (firstBinBuilt) {
           buildRecompressor();
         }

         firstBinBuilt = true;
       });
}

function buildJpegturbo() {
  console.log(chalk.blue('✓ jpeg turbo downloaded successfully'));
  exec('cd src/jpeg-turbo && autoreconf -fiv && ./configure && make',
       function(err, stdout, stderr) {
        if (err) {
          exitOnError('Unable to build jpeg-turbo. ', stderr);
        }

        console.log(chalk.blue('✓ jpeg-turbo built successfully.'));

        if (firstBinBuilt) {
          buildRecompressor();
        }

        firstBinBuilt = true;
      });
}

function buildRecompressor() {
  exec('./configure -Lsrc/jpeg-turbo/.libs -Isrc/jpeg-turbo && make',
       function(err, stdout, stderr) {
         if (err) {
           exitOnError('Unable to build image recompressor.', stderr);
         }

         if (process.platform == 'darwin') {
           fs.copySync('src/mozjpeg/.libs/libturbojpeg.0.dylib', 'libmozjpeg.dylib');
         } else {
           fs.copySync('src/mozjpeg/.libs/libturbojpeg.so.0.1.0', 'libmozjpeg.so');
         }

         console.log(chalk.green('\n✓✓✓ image recompressor built successfully ✓✓✓'));
         process.exit(0);
       });
}
