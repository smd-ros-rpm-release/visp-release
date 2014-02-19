/****************************************************************************
 *
 * $Id: servoSimuSphere2DCamVelocitySecondaryTask.cpp 2457 2010-01-07 10:41:18Z nmelchio $
 *
 * This file is part of the ViSP software.
 * Copyright (C) 2005 - 2014 by INRIA. All rights reserved.
 * 
 * This software is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * ("GPL") version 2 as published by the Free Software Foundation.
 * See the file LICENSE.txt at the root directory of this source
 * distribution for additional information about the GNU GPL.
 *
 * For using ViSP with software that can not be combined with the GNU
 * GPL, please contact INRIA about acquiring a ViSP Professional 
 * Edition License.
 *
 * See http://www.irisa.fr/lagadic/visp/visp.html for more information.
 * 
 * This software was developed at:
 * INRIA Rennes - Bretagne Atlantique
 * Campus Universitaire de Beaulieu
 * 35042 Rennes Cedex
 * France
 * http://www.irisa.fr/lagadic
 *
 * If you have questions regarding the use of this file, please contact
 * INRIA at visp@inria.fr
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 *
 * Description:
 * Simulation of a 2D visual servoing on a sphere.
 *
 * Authors:
 * Eric Marchand
 * Fabien Spindler
 *
 *****************************************************************************/


/*!
  \example servoSimuSphere2DCamVelocitySecondaryTask.cpp
  Servo a sphere:
  - eye-in-hand control law,
  - velocity computed in the camera frame,
  - without display,
  - a secondary task is the added.

*/

#include <stdlib.h>
#include <stdio.h>

#include <visp/vpFeatureBuilder.h>
#include <visp/vpFeatureEllipse.h>
#include <visp/vpHomogeneousMatrix.h>
#include <visp/vpMath.h>
#include <visp/vpParseArgv.h>
#include <visp/vpRobotCamera.h>
#include <visp/vpServo.h>
#include <visp/vpSphere.h>
#include <visp/vpSimulatorCamera.h>

// List of allowed command line options
#define GETOPTARGS	"h"

void usage(const char *name, const char *badparam);
bool getOptions(int argc, const char **argv);

/*!

Print the program options.

  \param name : Program name.
  \param badparam : Bad parameter name.

*/
void usage(const char *name, const char *badparam)
{
  fprintf(stdout, "\n\
Simulation of a 2D visual servoing on a sphere:\n\
- eye-in-hand control law,\n\
- velocity computed in the camera frame,\n\
- without display,\n\
- a secondary task is the added.\n\
          \n\
SYNOPSIS\n\
  %s [-h]\n", name);

  fprintf(stdout, "\n\
OPTIONS:                                               Default\n\
                  \n\
  -h\n\
     Print the help.\n");

  if (badparam)
    fprintf(stdout, "\nERROR: Bad parameter [%s]\n", badparam);
}

/*!

Set the program options.

  \param argc : Command line number of parameters.
  \param argv : Array of command line parameters.

  \return false if the program has to be stopped, true otherwise.

*/
bool getOptions(int argc, const char **argv)
{
  const char *optarg_;
  int	c;
  while ((c = vpParseArgv::parse(argc, argv, GETOPTARGS, &optarg_)) > 1) {

    switch (c) {
    case 'h': usage(argv[0], NULL); return false; break;

    default:
      usage(argv[0], optarg_);
      return false; break;
    }
  }

  if ((c == 1) || (c == -1)) {
    // standalone param or error
    usage(argv[0], NULL);
    std::cerr << "ERROR: " << std::endl;
    std::cerr << "  Bad argument " << optarg_ << std::endl << std::endl;
    return false;
  }

  return true;
}

int
main(int argc, const char ** argv)
{
  try {
    // Read the command line options
    if (getOptions(argc, argv) == false) {
      exit (-1);
    }

    vpServo task ;
    vpSimulatorCamera robot ;

    std::cout << std::endl ;
    std::cout << "-------------------------------------------------------" << std::endl ;
    std::cout << " Test program for vpServo "  <<std::endl ;
    std::cout << " Simulation " << std::endl ;
    std::cout << " task : servo a sphere with a secondary task" << std::endl ;
    std::cout << "-------------------------------------------------------" << std::endl ;
    std::cout << std::endl ;


    // sets the initial camera location
    vpHomogeneousMatrix cMo ;
    cMo[0][3] = 0.1 ;
    cMo[1][3] = 0.2 ;
    cMo[2][3] = 2 ;
    // Compute the position of the object in the world frame
    vpHomogeneousMatrix wMc, wMo;
    robot.getPosition(wMc) ;
    wMo = wMc * cMo;

    vpHomogeneousMatrix cMod ;
    cMod[0][3] = 0 ;
    cMod[1][3] = 0 ;
    cMod[2][3] = 1 ;

    // sets the sphere coordinates in the world frame
    vpSphere sphere ;
    sphere.setWorldCoordinates(0,0,0,0.1) ;

    // sets the desired position of the visual feature
    vpFeatureEllipse pd ;
    sphere.track(cMod) ;
    vpFeatureBuilder::create(pd,sphere)  ;

    // computes  the sphere coordinates in the camera frame and its 2D coordinates
    // sets the current position of the visual feature
    vpFeatureEllipse p ;
    sphere.track(cMo) ;
    vpFeatureBuilder::create(p,sphere)  ;

    // define the task
    // - we want an eye-in-hand control law
    // - robot is controlled in the camera frame
    task.setServo(vpServo::EYEINHAND_CAMERA) ;

    // we want to see a sphere on a sphere
    std::cout << std::endl ;
    task.addFeature(p,pd) ;

    // set the gain
    task.setLambda(1) ;

    // Display task information
    task.print() ;
    // exit(1) ;
    unsigned int iter=0 ;
    // loop
    while(iter++ < 500)
    {
      std::cout << "---------------------------------------------" << iter <<std::endl ;
      vpColVector v ;

      // get the robot position
      robot.getPosition(wMc) ;
      // Compute the position of the camera wrt the object frame
      cMo = wMc.inverse() * wMo;

      // new sphere position: retrieve x,y and Z of the vpSphere structure
      sphere.track(cMo) ;
      vpFeatureBuilder::create(p,sphere);

      vpColVector de2dt(6) ;
      de2dt[2] = 1 ;    // should be zero in (I-WpW)de2dt
      de2dt[5] = 0.01 ; // should be ok
      de2dt[0] = 0.01 ;  // should generate a motion on (I-WpW)de2dt[4]

      // compute the control law
      v = task.computeControlLaw() ;

      std::cout << "de2dt :"<< de2dt.t() << std::endl;
      vpColVector sec ;
      sec = task.secondaryTask(de2dt) ;
      std::cout << "(I-WpW)de2dt :"<< sec.t() << std::endl;

      if (iter>20)  v += sec ;

      // send the camera velocity to the controller
      robot.setVelocity(vpRobot::CAMERA_FRAME, v) ;

      std::cout << "|| s - s* || = " << ( task.getError() ).sumSquare() <<std::endl ;
    }

    // Display task information
    task.print() ;
    task.kill();
    return 0;
  }
  catch(vpException e) {
    std::cout << "Catch a ViSP exception: " << e << std::endl;
    return 1;
  }
}

