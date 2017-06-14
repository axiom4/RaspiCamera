<?php

namespace GPhotoBundle\Model;

class GPhotoEntry {
	/**
	* @var number
	*/
	public $id;

	/**
	* @var string
	*/
	public $filename;

	/**
	* @var string
	*/
	public $filetype;

    /**
	* @var string
	*/
	public $filesize;

    /**
	* @var string
	*/
	public $fileres;

	public function __construct(
        $id = 0, 
        $filename = "",
        $filetype = "",
        $filesize = "",
        $fileres = ""
    ) {
        $this->id = intval($id);
		$this->filename = $filename;
		$this->filetype = $filetype;
		$this->filesize = $filesize;
		$this->fileres = $fileres;
		
		return $this;
	}
}
