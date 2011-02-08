<?php
// $Id: plugins.inc,v 1.1.2.1 2011/01/09 18:13:32 smoothify Exp $

/**
 * Call all Dependent Fields hooks stored in the available include files.
 */
function dependent_fields_invoke_all($method, &$params) {
  $return = array();
  foreach (dependent_fields_includes() as $source) {
    $function = 'dependent_fields_' . $source . '_' . $method;
    if (function_exists($function)) {
      $result = call_user_func_array($function, $params);
      if (isset($result) && !empty($result) && is_array($result)) {
        $return = array_merge_recursive($return, $result);
      }
      elseif (isset($result)) {
        $return[] = $result;
      }
    }
  }
  return $return;
}

/**
 * Call Dependent Fields hooks for specific fields / widgets
 */
function dependent_fields_invoke_field($field, $method, &$params) {
  $field_name = $field;
  $return = array();
  $info = dependent_fields_info();

  // Make sure we can handle this field.
  if (empty($info[$field_name])) {
    return $return;
  }
  $field_info = $info[$field_name];

  // Create and test if the function name exists.
  $function = 'dependent_fields_' . $field_name . '_' . $method;
  if (function_exists($function)) {
    $result = call_user_func_array($function, $params);
    if (isset($result) && !empty($result)) {
      $return = $result;
    }
  }

  return $return;
}

/**
 * Load hook_dependent_fields_info() data from all modules.
 */
function dependent_fields_info() {
  static $info = array();
  if (!empty($info)) {
    return $info;
  }
  $info = module_invoke_all('dependent_fields_info');
  drupal_alter('dependent_fields_info', $info);
  return $info;
}

/**
 * Create a list of Dependent Fields by name, suitable for a select list.
 */
function dependent_fields_list($include_default = TRUE) {
  $info = dependent_fields_info();
  $list = array();

  foreach ($info as $key => $source) {
    $list[$key] = $source['name'];
  }

  return $list;
}

/**
 * Implementation of hook_dependent_fields_info().
 */
function dependent_fields_dependent_fields_info() {
  $params = array();
  return dependent_fields_invoke_all('info', $params);
}

/**
 * Load all the potential sources.
 */
function dependent_fields_includes($include = TRUE) {
  static $includes = array();
  if (!empty($includes)) {
    return $includes;
  }

  $directory = drupal_get_path('module', 'dependent_fields') . '/fields';
  foreach (file_scan_directory($directory, '.inc$') as $file) {
    $includes[] = $file->name;
    if ($include) {
      include_once($file->filename);
    }
  }
  return $includes;
}

/**
 * This returns a list of widgets that are compatible with Dependent Fields.
 */
function dependent_fields_allowed_widgets($field = '', $type = 'both') {
  $widgets = array();
  foreach (dependent_fields_info() as $source => $info) {
    if (empty($field) || $field == $source) {
      if ($type == 'both' || $type == 'controller') {
        if (!empty($info['controller_widgets'])) {
          $widgets = array_merge_recursive($widgets, $info['controller_widgets']);
        }
      }
      if ($type == 'both' || $type == 'controlled') {
        if (!empty($info['controlled_widgets'])) {
          $widgets = array_merge_recursive($widgets, $info['controlled_widgets']);
        }
      }
    }
  }
  return array_unique($widgets);
}

function dependent_fields_build_field_info($field, $form_state = NULL, $controller_value = NULL, $omit_field_value = FALSE) {
  $info = array();
  if (!empty($field)) {
    $type = (array)content_types($field['type_name']);
    $controller_field = dependent_fields_controller_field($field, $field['type_name']);
    $controlled_fields = dependent_fields_controlled_fields($field, $type);
    $hide_if_empty = $field['widget']['dependent_fields']['hide_if_empty'];
    $hide_none = $field['widget']['dependent_fields']['hide_none'];
    // Check to see if we have either a controlled or controller field
    // If so, there is nothing to do here.
    if (empty($controller_field) && empty($controlled_fields)) {
      return $info;
    }
    $info = array(
      'field' => $field,
      'widget' => $field['widget'],
      'hide_if_empty' => $hide_if_empty,
      'hide_none' => $hide_none,

    );
    if (!$omit_field_value) {
      $info['field_value'] = dependent_fields_retrieve_field_value($field, $form_state);
    }

    if (!empty($controller_field)) {
      $info['controller_field'] = $controller_field;
      // Check to see if a controller value has been passed in.
      if (!empty($controller_value)) {
        $info['controller_value'] = $controller_value;
      }
      else {
        $info['controller_value'] = dependent_fields_retrieve_field_value($controller_field, $form_state);
      }
    }
    if (!empty($controlled_fields)) {
      $info['controlled_fields'] = $controlled_fields;
    }
  }
  $params = array($info, $field, $field['widget'], $type);
  $info = array_merge($info, dependent_fields_invoke_field($field['type'], 'build_field_info', $params));

  return $info;
}