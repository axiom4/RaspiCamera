import { Component, OnInit } from '@angular/core';
import { Observable }        from 'rxjs/Observable';

import { RaspiCameraGphotoService } from '../raspi-camera-gphoto.service';
import { PhotoEntry } from '../photo-entry';

import 'rxjs/add/observable/of';

@Component({
  selector: 'app-preview',
  templateUrl: './preview.component.html',
  styleUrls: ['./preview.component.css'],
  providers: [RaspiCameraGphotoService]

})
export class PreviewComponent implements OnInit {
  photos: PhotoEntry[];

  constructor(private raspiCameraGphotoService: RaspiCameraGphotoService) { }

  ngOnInit() {
    this.raspiCameraGphotoService.getCameraPhotos()
      .subscribe(photos => {
        this.photos = photos
      })
  }

}
