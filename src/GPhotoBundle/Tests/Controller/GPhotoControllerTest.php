<?php

namespace GPhotoBundle\Tests\Controller;

use Symfony\Bundle\FrameworkBundle\Test\WebTestCase;

class GPhotoControllerTest extends WebTestCase
{
    public function testListphoto()
    {
        $client = static::createClient();

        $crawler = $client->request('GET', '/listPhoto');
    }

    public function testGetphotobyid()
    {
        $client = static::createClient();

        $crawler = $client->request('GET', '/api/getPhotoById');
    }

    public function testGetphotopreviewbyid()
    {
        $client = static::createClient();

        $crawler = $client->request('GET', '/getPhotoPreviewById');
    }

}
