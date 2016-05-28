//
//  InstrumentsServiceController.m
//  IceStar
//
//  Created by Andreas Müller on 28.05.16.
//  Copyright © 2016 Andreas Müller. All rights reserved.
//

#import "InstrumentsServiceController.h"
#import "InstrumentsViewController.h"

@implementation InstrumentsServiceController

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
    NSLog(@"connect to %ld", (long)indexPath.row);
    [(InstrumentsViewController *)_instrumentsViewController connectInstruments: indexPath.row];
}


@end
