import { GphotoPage } from './app.po';

describe('gphoto App', () => {
  let page: GphotoPage;

  beforeEach(() => {
    page = new GphotoPage();
  });

  it('should display welcome message', () => {
    page.navigateTo();
    expect(page.getParagraphText()).toEqual('Welcome to app!!');
  });
});
