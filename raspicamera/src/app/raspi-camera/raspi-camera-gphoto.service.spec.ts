import { TestBed, inject } from '@angular/core/testing';

import { RaspiCameraGphotoService } from './raspi-camera-gphoto.service';

describe('RaspiCameraGphotoService', () => {
  beforeEach(() => {
    TestBed.configureTestingModule({
      providers: [RaspiCameraGphotoService]
    });
  });

  it('should be created', inject([RaspiCameraGphotoService], (service: RaspiCameraGphotoService) => {
    expect(service).toBeTruthy();
  }));
});
