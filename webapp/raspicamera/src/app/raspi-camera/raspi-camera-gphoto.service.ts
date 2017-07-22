import { Injectable } from '@angular/core';

import { Http, Response } from '@angular/http';

import { Observable } from 'rxjs/Observable';
import 'rxjs/add/operator/map';

import { PhotoEntry } from './photo-entry';

@Injectable()
export class RaspiCameraGphotoService {

  private baseUrl: string = '/api/photos';

  constructor(private http: Http) { }

  getCameraPhotos(): Observable<PhotoEntry[]> {
    return this.http.get(this.baseUrl)
      .map(response => response.json() as PhotoEntry[]);
  }

}
