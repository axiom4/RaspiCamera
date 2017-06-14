<?php

namespace RaspiCameraBundle\Service;

use RaspiCameraBundle\Model\GPhotoEntry;

class RaspiCameraService {
    private $gphoto_exec;

    public function __construct($kernel) {
        $this->gphoto_exec = $kernel->getContainer()->getParameter('gphoto2_path');
    }

    public function fetchGPhotoList() {
        $response = array();
        exec($this->gphoto_exec . " --list-files", $output);

        foreach ($output as $line) {
            if(preg_match("/^#(?<id>[0-9]+)\s+(?<filename>[\w\.]+)\s+rd\s+(?<filesize>[0-9]+ KB)\s+(?<fileres>[0-9x]*)\s*(?<filetype>[\w\/-]+)$/", $line, $match)) {
                //#1     IMG_0043.JPG               rd  4276 KB 3724x2096 image\/jpeg"
                //#1     O58A6571.JPG               rd  7469 KB image/jpeg
                $entry = new GPhotoEntry(
                    $match['id'],
                    $match['filename'],
                    $match['filetype'],
                    $match['filesize'],
                    $match['fileres']
                );

                array_push($response, $entry);
            }
        }

        return $response;
    }

    public function fetchGPhotoFile($id) {
        $command = $this->gphoto_exec . " --stdout --get-file=$id 2>&1";

        $fp = popen($command, "r");

        $var = null;

        while(!feof($fp)) { 
            $r = fread($fp, 4096);

            if(!$var) {
                $var = $r;
            } else {
                $var .= $r;
            }
        } 
        fclose($fp);

        return $var;
    }

    public function fetchGPhotoPreview($id) {
        $command = $this->gphoto_exec . " --stdout --get-thumbnail=$id 2>&1";

        $fp = popen($command, "r");

        $var = null;

        while(!feof($fp)) { 
            $r = fread($fp, 4096);

            if(!$var) {
                $var = $r;
            } else {
                $var .= $r;
            }
        } 
        fclose($fp);

        return $var;
    }

    public function fetchGPhotoInfo($id) {
        $command = $this->gphoto_exec . " --show-info=$id";

        $fp = popen($command, "r");

        $var = null;

        while(!feof($fp)) { 
            $r = fread($fp, 4096);

            if(!$var) {
                $var = $r;
            } else {
                $var .= $r;
            }
        } 
        fclose($fp);

        return $var;
    }

    public function fetchGPhotoMimeType($id) {
        $command = $this->gphoto_exec . " --show-info=$id";
        $mimetype = "application/x-unknown";
        $filename = "";
        $fetch = 0;

        exec($command, $output);

        foreach ($output as $line) {
            if(preg_match("/^\s+Mime type:\s+'(?<mimetype>[\w\/-]+)'/", $line, $match) && !$fetch) {
                $mimetype = $match['mimetype'];
                $fetch = 1;
            } else if(preg_match("/^Information on file '(?<filename>[\w\.]+)'.*:$/", $line, $match)) {
                $filename = $match['filename'];
            }
        }
        $obj = array('filename' => $filename, 'mimetype' => $mimetype);

        return json_encode($obj);
    }

    public function fetchGPhotoExif($id) {
        $command = $this->gphoto_exec . " --show-exif=$id";

        $fp = popen($command, "r");

        $var = null;

        while(!feof($fp)) { 
            $r = fread($fp, 4096);

            if(!$var) {
                $var = $r;
            } else {
                $var .= $r;
            }
        } 
        fclose($fp);

        return $var;
    }


}
