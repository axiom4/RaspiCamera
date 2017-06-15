<?php

namespace RaspiCameraBundle\Controller;

use Symfony\Bundle\FrameworkBundle\Controller\Controller;
use Sensio\Bundle\FrameworkExtraBundle\Configuration\Route;

use Symfony\Component\HttpFoundation\Request;
use Symfony\Component\HttpFoundation\Response;
use Symfony\Component\HttpFoundation\ResponseHeaderBag;
use Symfony\Component\HttpFoundation\BinaryFileResponse;
use Symfony\Component\HttpFoundation\StreamedResponse;


class RaspiCameraController extends Controller
{
    /**
     * @Route("/api/photos", name="listPhotos")
     */
    public function listPhotosAction(Request $request) {
        $new = ($request->query->has('new') ? 1 : 0);

		$response = new Response();
		
		$gphoto_service = $this->get('raspicamera.raspicamera_service');
		
		$var = $gphoto_service->fetchGPhotoList($new);
		
		$response->setContent(json_encode($var));
		
		$response->headers->set('Content-Type', 'application/json');

		return $response;
    }

    /**
     * @Route("/api/photos/download/{id}", name="getPhotoById")
     */
    public function getPhotosByIdAction(Request $request, $id) {
		
		$gphoto_service = $this->get('raspicamera.raspicamera_service');
		
		$fileContent = $gphoto_service->fetchGPhotoFile($id);
        $filemime = json_decode($gphoto_service->fetchGPhotoMimeType($id));

        $filename = $filemime->{'filename'};
        $mimetype = $filemime->{'mimetype'};

		$response = new Response($fileContent);
        $response->headers->set('Content-Type', $mimetype);

        $disposition = $response->headers->makeDisposition(ResponseHeaderBag::DISPOSITION_INLINE, $filename);
        $response->headers->set('Content-Disposition', $disposition);

        return $response;
    }

    /**
     * @Route("/api/photos/preview/{id}", name="getPhotoPreviewById")
     */
    public function getPhotoPreviewByIdAction(Request $request, $id) {
		
		$gphoto_service = $this->get('raspicamera.raspicamera_service');
		
		$fileContent = $gphoto_service->fetchGPhotoPreview($id);
        $filemime = json_decode($gphoto_service->fetchGPhotoMimeType($id));

        $info = pathinfo($filemime->{'filename'});

        $filename = "thumb_".$info['filename'].".jpg";
        $mimetype = $filemime->{'mimetype'};

		$response = new Response($fileContent);
        $response->headers->set('Content-Type', 'image/jpeg');

        $disposition = $response->headers->makeDisposition(ResponseHeaderBag::DISPOSITION_INLINE, $filename);
        $response->headers->set('Content-Disposition', $disposition);
        
        return $response;
    }

    /**
     * @Route("/api/photos/info/{id}", name="getPhotoInfoById")
     */
    public function getPhotoInfoByIdAction(Request $request, $id) {
		
		$gphoto_service = $this->get('raspicamera.raspicamera_service');
		
		$fileContent = $gphoto_service->fetchGPhotoInfo($id);

		$response = new Response($fileContent);
        $response->headers->set('Content-Type', 'text/plain');

        return $response;
    }

    /**
     * @Route("/api/photos/info/mimetype/{id}", name="getPhotoInfoMimeTypeById")
     */
    public function getPhotoInfoMimeTypeByIdAction(Request $request, $id) {
		
		$gphoto_service = $this->get('raspicamera.raspicamera_service');
		
		$fileContent = $gphoto_service->fetchGPhotoMimeType($id);

		$response = new Response($fileContent);
        $response->headers->set('Content-Type', 'application/json');

        return $response;
    }

    /**
     * @Route("/api/photos/exif/{id}", name="getPhotoExifById")
     */
    public function getPhotoExifByIdAction(Request $request, $id) {
		
		$gphoto_service = $this->get('raspicamera.raspicamera_service');
		
		$fileContent = $gphoto_service->fetchGPhotoExif($id);

		$response = new Response($fileContent);
        $response->headers->set('Content-Type', 'text/plain');

        return $response;
    }

}
