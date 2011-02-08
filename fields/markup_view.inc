<?php
// $Id: markup_view.inc,v 1.1.2.2 2011/01/09 18:13:32 smoothify Exp $

/**
 * @file
 * A Dependent Fields extension for the markup view field
 *
 * The "hooks" in this file are not true hooks, they're called individually
 * from the main dependent_fields.module in the corresponding hook by the
 * same name. Any of these hooks could be broken out into a separate module.
 */

/**
 * Implementation of hook_dependent_fields_info().
 */
function dependent_fields_markup_view_info() {
  $field = array();
  $field['markup_view'] = array(
    'name' => t('Markup View'),
    'controlled_widgets' => array(
      'markup_view',
    ),
  );
  return $field;
}


/**
 * Implementation of hook_dependent_fields_widget_form().
 */
function dependent_fields_markup_view_widget_form($widget, $field, $type) {
  $form = array();
  $view_args = isset($widget['dependent_fields']['view_args']) ? $widget['dependent_fields']['view_args'] : '';

  $form['view_args'] = array(
    '#type' => 'textfield',
    '#title' => t('View arguments'),
    '#default_value' => $view_args,
    '#description' => t('Provide a comma separated list of arguments to pass to the view.'),
  );

  $form['controller_tokens_help'] = array(
    '#value' => dependent_fields_string_tokens_help(),
  );

  return $form;
}

/**
 * Implementation of hook_dependent_fields_build_field_info().
 */
function dependent_fields_markup_view_build_field_info($info, $field, $widget, $type) {
  $return = array();
  $view_args = isset($widget['dependent_fields']['view_args']) ? $widget['dependent_fields']['view_args'] : '';
  $return['view_args'] = $view_args;
  return $return;
}

/**
 * A preprocess callback for the markup_view form element
 */
function dependent_fields_markup_view_element_preprocess($element, $info, $form_state, $form) {
  if (!empty($info['controller_field']) && !empty($info['view_args'])) {
    $element['#argument_callback'] = 'dependent_fields_markup_view_get_arguments';
  }
  return $element;
}

function dependent_fields_markup_view_get_arguments($element, $field, $form_state, $form) {
  $args = array();
  $info = dependent_fields_build_field_info($field, $form_state);
  if (!empty($info)) {
    $view_args = $info['view_args'];
    $args = dependent_fields_replace_string_tokens($info, $view_args);
  }
  return $args;
}
