<?php
// $Id: form.inc,v 1.1.2.10 2011/01/28 11:32:22 smoothify Exp $

/**
 * Implementation of node_form_alter - invoked in hook_form_alter
 */

function dependent_fields_node_form_alter(&$form, &$form_state, $form_id) {
  // Get any controller fields.
  $type = (array)content_types($form['type']['#value']);
  $controllers = dependent_fields_type_controllers($type);

  // Only add the callbacks and overrides if we have at least one controller field.
  if (!empty($controllers)) {
    dependent_fields_set_form_state($form_state);
    dependent_fields_set_field_overrides($form['#field_info'], $controllers);
    dependent_fields_reorganize_form($form, $controllers, $type['fields'], $type);
    $form['#after_build'][] = 'dependent_fields_node_form_after_build';
    $form['#pre_render'][] = 'dependent_fields_node_form_pre_render';
    $form['#validate'][] = 'dependent_fields_node_form_validate';
  }
}

/**
 * Additional #after_build handler for the node form.
 *
 * This function sets the caching status to true, so we can retrieve it in our
 * AHAH callback. It also moves any dependent fields into the controller field's
 * AHAH wrapper.
 */

function dependent_fields_node_form_after_build($form, &$form_state) {
  // Add CSS file to set the form styles.
  dependent_fields_add();
  $form['#cache'] = TRUE;
  $type = (array)content_types($form['type']['#value']);

  $controllers = dependent_fields_type_controllers($type);

  // Check to see if this type has any controller fields.
  if (!empty($controllers)) {
    // Check to see if this is an ahah submission.
    if (!empty($form_state['ahah_submission']) && $form_state['ahah_submission']) {
      // Disable validation on the entire form, to prevent errors on fields.
      dependent_fields_disable_validation($form);
    }
    // To be on the safe site, reset the field overrides
    dependent_fields_set_field_overrides($form['#field_info'], NULL, TRUE);
  }

  return $form;
}

/**
 * Additional #validate handler for the node form.
 */
function dependent_fields_node_form_validate($form, &$form_state) {
  $continue_button = $form_state['clicked_button']['#submit'][0] == 'dependent_fields_fallback_submit' ? TRUE : FALSE;
    if ($continue_button) {
      /// Since we can't seem to disable all validation, lets clear error messages
      drupal_get_messages();
    }
}

/**
 * Additional #pre_render handler for the node form.
 */

function dependent_fields_node_form_pre_render($form) {
  // Add CSS file to set the form styles.
  dependent_fields_add();
  return $form;
}

/**
 * Run the after build functions on the field element.
 */
function dependent_fields_callback_after_build(&$form, &$form_state) {
  if (!is_array($form)){
    return;
  }

  if (isset($form['#after_build'])) {
    foreach ($form['#after_build'] as $function) {
      $form = $function($form, $form_state);
      $form['#after_build']['#after_build_done'] = TRUE;
    }
  }
  foreach ($form as $id => $element) {
    if ($id != '#after_build' && is_array($element)) {
      dependent_fields_callback_after_build($form[$id], $form_state);
    }
  }
}

/**
 * Disable validation for all of the form.
 */
function dependent_fields_disable_validation(&$form) {
  foreach (element_children($form) as $child) {
    $form[$child]['#validated'] = TRUE;
    $form[$child]['#element_validate'] = NULL;
    $form[$child]['#needs_validation'] = FALSE;
    dependent_fields_disable_validation($form[$child]);
  }
}

/**
 * Function to prepare form to work with the ahah callback
 */
function dependent_fields_reorganize_form(&$form, $tree, $fields, $type, $level = 0) {
  foreach ($tree as $field_name => $field_array) {
    $controller_field = $field_name;
    $child_count = count($field_array);
    // Check if there are any controlled fields.
    if ($child_count ==  0) {
      continue;
    }
    // Recursive function to reorder controlled fields that are also controllers.
    dependent_fields_reorganize_form($form, $field_array, $fields, $type, $level + 1);

    $group_name = &dependent_fields_get_field_group_name($type, $field_name);
 
    if (!empty($group_name)) {
      $parent = &$form[$group_name];
    }
    else {
      $parent = &$form;
    }

    if (empty($parent) || empty($parent[$field_name])) {
      continue;
    }

    // Override add more ahah path.
    dependent_fields_override_add_more_path($parent, $field_name, $type['type']);
    
    $controller = array();
    $controller['#type'] = 'dependent_controller';
    $controller['#controller_field_name'] = $field_name;
    $controller[$field_name] = $parent[$field_name];
    $controller['#weight'] = $controller[$field_name]['#weight'];

    // if there is only one controlled field, we don't need to move the field.
    if ($child_count == 1 && $level == 0) {
      reset($field_array);
      $controlled_field = key($field_array);
      $controlled_parent = &dependent_fields_retrieve_field_parent_element($form, $type, $controlled_field);
      $controller['#controlled_field'] = $controlled_field;
      $wrapper = array();
      $wrapper['#type'] = 'dependent_wrapper';
      $wrapper['#parent_field'] = $field_name;
      $wrapper['#field_count'] = $child_count;
      $wrapper['#fields'] = array();
      $wrapper['#fields'][$controlled_field] = $controlled_parent[$controlled_field];
      $controlled_parent[$field_name . '_ahah_wrapper'] = $wrapper;
      unset($controlled_parent[$controlled_field]);

    }
    // If there are multiple fields, or we are in a cascading dependency, move fields to parent.
    else {
      $controller['#fields'] = array();
      $group_array = array();
      $field_count = 0;

      foreach ($field_array as $controlled_field => $child_fields) {
        // Check if the field is part of a group.
        $group_name = dependent_fields_get_field_group_name($type, $controlled_field);
        if (!empty($group_name)) {
          // Add group_name to the array for later processing.
          $group_array[$group_name] = $group_name;
        }
        else {
          // Just a standard field with no group.
          ++$field_count;
          // Override ahah add_more path
          dependent_fields_override_add_more_path($form, $controlled_field, $form['type']['#value']);
          $controlled_parent = $form[$controlled_field];
          $controller['#field_count'] = $field_count;
          $controller['#fields'][$controlled_field] = $form[$controlled_field];
          unset($form[$controlled_field]);
        }
      }
      $group_count = count($group_array);
      // Check if we are dealing with a single group, if so we don't need to move it, just enclose in a wrapper.
      if ($field_count == 0 && $group_count == 1) {
         reset($group_array);
         $controlled_group = key($group_array);
         $controller['#controlled_group'] = $controlled_group;
         $wrapper = array();
         $wrapper['#type'] = 'dependent_wrapper';
         $wrapper['#parent_field'] = $field_name;
         $wrapper['#field_count'] = $group_count;
         $wrapper['#group_name'] = $controlled_group;
         $wrapper['#fields'] = array();
         foreach (element_children($form[$controlled_group]) as $controlled_field => $element) {
           // Override ahah add_more paths.
           dependent_fields_override_add_more_path($form[$controlled_group], $controlled_field, $form['type']['#value']);
           $wrapper['#fields'][$element] = $form[$controlled_group][$element];
           unset($form[$controlled_group][$element]);
         }
         $form[$controlled_group][$field_name . '_ahah_wrapper'] = $wrapper;
      }
      // We have multiple items so lets move the groups too.
      elseif (!empty($group_count)) {
        foreach ($group_array as $group_name) {
          // move entire group to controller wrapper.
          foreach (element_children($form[$group_name]) as $controlled_field => $element) {
           // Override ahah add_more paths.
           dependent_fields_override_add_more_path($form[$group_name], $controlled_field, $form['type']['#value']);
          }
          $controller['#field_count'] = $group_count;
          $controller['#fields'][$group_name] = $form[$group_name];
          unset($form[$group_name]);
        }
      }
    }
    // Set the controller element to our new wrapper.
    $parent[$field_name] = $controller;
  }
}

/**
 * Function to check if a field is located within a fieldgroup.
 */
function dependent_fields_get_field_group_name($type, $field_name) {
  if (module_exists('fieldgroup') && ($group_name = _fieldgroup_field_get_group($type['type'], $field_name))) {
    return $group_name;
  }

  return FALSE;
}

/**
 * Function to retrieve a field's parent element from a form.
 */
function &dependent_fields_retrieve_field_parent_element(&$form, $type, $field_name) {
  $group_name = &dependent_fields_get_field_group_name($type, $field_name);

  if (!empty($group_name)) {
    $parent = &$form[$group_name];
  }
  else {
    $parent = &$form;
  }

  return $parent;
}

/**
 * Function to retrieve a field element from a form.
 */
function &dependent_fields_retrieve_field_element(&$form, $type, $field_name) {
  $parent = &dependent_fields_retrieve_field_parent_element($form, $type, $field_name);
  return $parent[$field_name];
}


/**
 * Function to override an elements add more ahah wrapper.
 */
function dependent_fields_override_add_more_path(&$form, $field_name, $type) {
  // Set add more overrides if add more button exists.
  if (!empty($form[$field_name][$field_name . '_add_more'])) {
    $path = 'dependent_fields/js_add_more/' . $type . '/' . $field_name;
    $form[$field_name][$field_name . '_add_more']['#ahah']['path'] = $path;
  }
}


/**
 * Function to retrieve a field element form from a form.
 */
function &dependent_fields_retrieve_field_form_parent(&$form, $field, $type) {
  $field_name = $field['field_name'];
  $group_name = dependent_fields_get_field_group_name($type, $field_name);
  $controller_field = dependent_fields_controller_field($field, $type);
  $controlled_fields = dependent_fields_controlled_fields($field, $type);

  // Check to find out which kind of field this is.
  if (empty($controller_field) && empty($controlled_fields)) {
    // This field is not managed by dependent fields so we can simply return a regular field.
    $parent = &dependent_fields_retrieve_field_parent_element($form, $field_name, $type);
    return $parent;
  }
  elseif (empty($controller_field)) {
    if (!empty($group_name)) {
      $parent = &$form[$group_name][$field_name];
    }
    else {
      $parent = &$form[$field_name];
    }
    return $parent;
  }
  else {
    // We have controller field, so the field could be in one of several places.
    $controller_field_name = $controller_field['field_name'];

    if (!empty($group_name)) {
      $controller_field_name = $controller_field['field_name'];

      if (!empty($form[$group_name][$controller_field_name . '_ahah_wrapper'])) {
        $parent = $form[$group_name][$controller_field_name . '_ahah_wrapper'];
      }
      else {
        $parent = &dependent_fields_retrieve_field_form_parent($form, $controller_field, $type);
      }
    }
    else {
      // We have controller field, so the field could be in one of several places.
      $parent_controlled_fields = dependent_fields_controlled_fields_list($controller_field, $type);
      $parent_controller_field = dependent_fields_controller_field($controller_field, $type);
      // if there is only one controlled field, then the field is not moved.
      if (count($parent_controlled_fields) == 1 && empty($parent_controller_field)) {
        $parent = &$form[$controller_field_name . '_ahah_wrapper'];
      }
      else {
        $parent = &dependent_fields_retrieve_field_form_parent($form, $controller_field, $type);
      }
    }

    if ($parent['#processed'] && !empty($parent[$field_name])) {
      $parent = &$parent[$field_name];
    }
    else {
      $parent = &$parent['#fields'];
    }
    
    if (!empty($controlled_fields)) {
      // Multi level dependency, lets find the correct field element.
      $parent = &$parent[$field_name];
    }
    return $parent;
  }

}

/**
 * Function to retrieve a field element form from a form.
 */
function dependent_fields_retrieve_field_form(&$form, $field, $type) {
  $field_name = $field['field_name'];
  $parent = dependent_fields_retrieve_field_form_parent($form, $field, $type);
  $field_element = $parent[$field_name];
  return $field_element;
}