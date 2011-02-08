// $Id: README.txt,v 1.1.2.2 2011/01/09 18:13:32 smoothify Exp $

Description
-----------
The AHAH Dependent Fields module allows cck based fields to specify 
another field as a controller field, and react to its changes on the 
form using AHAH without the form re-loading.

A common use case for this will be Dependent Node Reference fields, 
where the dependent field changes depending on what is selected in the 
Controller field. This behaviour will be supported, in fact that was the
primary reason for starting development.

Author & Sponsor
----------------
AHAH Dependent Fields was written by Ben Osman.

This module was created by Radiant Flow: http://radiantflow.com  and 
sponsored by Spirit Library: http://spiritlibrary.com

Warning
-------
This module is in the early stages of development - while most things should 
work, somethings WILL change and other things may be broken. Make sure you take
a backup first and do not use on a production site!


Dependencies
------------
 * content (part of CCK)
 * At least one field type from text, number & node reference

Install
-------
- Copy the unpacked folder "dependent_fields" in your modules folder.
- Go to the modules administration page (admin/build/modules) and activate the
  module (you will find it in the "CCK" package)

Usage
------
Once the module is activated, a dependent fields fieldset will appear in the
editing form of every cck field underneath the "Default value" fieldset.  

The path is admin/content/node-type/CONTENT_TYPE_NAME/fields/FIELD_NAME

Note that the options will not appear if there are no potential "controller" 
fields. 

A potential controller field must be one of the following types:

Node Reference Field
Text Field
Number Field

Limitations
-----------
- Each field may only have one controller field
- If a controller field has 2 or more dependent fields, then the module
  reorganizes the form to bring together the controlled fields underneath the
  controller field.
- If multi level dependencies are being used, i.e. and controlled field is also
  a controller, then reorganization with also take place.
 
To Do
-----

- More documentation + screencasts
- Add more fields, such as userreference
- Add more widgets such as Enhanced Node Reference, Text Autocomplete
- Improve and document the API, so developers can add their own custom
  functionality


Support
-------
Check the issue queue of this module for more information:
http://drupal.org/project/issues/dependent_fields

