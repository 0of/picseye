//
//  ViewController.m
//  picseye
//
//  Copyright © 2016 0of. All rights reserved.
//

#import "ViewController.h"
#import "opencv2/opencv.hpp"
#import "AFNetworking.h"

#define BASE_URLSTR @""

@interface ViewController ()<UINavigationControllerDelegate, UIImagePickerControllerDelegate>
@end

@implementation ViewController
- (void)viewDidLoad {
  [super viewDidLoad];
  // Do any additional setup after loading the view, typically from a nib.
}

- (void)didReceiveMemoryWarning {
  [super didReceiveMemoryWarning];
  // Dispose of any resources that can be recreated.
}

- (IBAction)onTakeSnapshot:(id)sender {
  UIImagePickerController *picker = [[UIImagePickerController alloc] init];
  picker.delegate = self;
  picker.sourceType = UIImagePickerControllerSourceTypeCamera;
  
  [[NSOperationQueue mainQueue] addOperationWithBlock:^{
    [self presentViewController:picker animated:YES completion:nil];
  }];
}

- (cv::Mat)cvMatGrayFromUIImage:(UIImage *)image {
  CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceGray();
  CGFloat cols = image.size.width;
  CGFloat rows = image.size.height;
  
  cv::Mat cvMat(rows, cols, CV_8UC1);
  CGContextRef contextRef = CGBitmapContextCreate(cvMat.data,     // Pointer to data
                                                  cols,           // Width of bitmap
                                                  rows,           // Height of bitmap
                                                  8,              // Bits per component
                                                  cvMat.step[0],  // Bytes per row
                                                  colorSpace,     // Color space
                                                  kCGImageAlphaNone
                                                  | kCGBitmapByteOrderDefault); // Bitmap info flags
  
  CGContextDrawImage(contextRef, CGRectMake(0, 0, cols, rows), image.CGImage);
  CGColorSpaceRelease(colorSpace);
  CGContextRelease(contextRef);
  
  return cvMat;
}

- (NSArray *)getDescriptors:(cv::Mat)pict {
  cv::ORB orb;
  
  std::vector<cv::KeyPoint> keyPoints;
  cv::Mat descriptors;
  orb(pict, cv::noArray(), keyPoints, descriptors);
  
  NSMutableArray *result = [NSMutableArray arrayWithCapacity:descriptors.rows];

  for(auto i = 0; i != descriptors.rows; ++i) {
    NSData *eachDesc = [NSData dataWithBytesNoCopy:descriptors.ptr<const uint8_t *>(i) length:32 freeWhenDone:NO];
    [result addObject:[eachDesc base64EncodedStringWithOptions:0]];
  }
  
  return result;
}

- (void)imagePickerController:(UIImagePickerController *)picker didFinishPickingImage:(UIImage *)image editingInfo:(NSDictionary *)editingInfo {
  [picker dismissViewControllerAnimated:YES completion:nil];
  
  NSArray *desc = [self getDescriptors:[self cvMatGrayFromUIImage:image]];
  
  AFHTTPSessionManager *manager = [[AFHTTPSessionManager alloc]initWithBaseURL:[NSURL URLWithString:BASE_URLSTR] sessionConfiguration:[NSURLSessionConfiguration defaultSessionConfiguration]];

  manager.requestSerializer = [AFJSONRequestSerializer serializer];
  [manager.requestSerializer setValue:@"application/json" forHTTPHeaderField:@"Content-Type"];
  
  NSDictionary *params = [NSDictionary dictionaryWithObjectsAndKeys:@"ORB", @"alg", desc, @"descriptors", nil];
  [manager POST:@"/search" parameters:params progress:nil success:^(NSURLSessionDataTask * _Nonnull task, id  _Nullable responseObject) {
    
    NSLog(@"success!");
  } failure:^(NSURLSessionDataTask * _Nullable task, NSError * _Nonnull error) {
    NSLog(@"error: %@", error);
  }];
}
@end
