<?php
// $Id: admin.inc,v 1.1.2.2 2011/01/20 09:56:53 smoothify Exp $

/**
 * Implementation of hook_content_edit_form().
 */
function dependent_fields_content_edit_form(&$form) {
  // Only support modules that implement hook_dependent_fields_widgets().
  $field = $form['#field'];
  $widget = $field['widget'];

  $widget_type = isset($widget['widget_type']) ? $widget['widget_type'] : $widget['type'];
  if (!in_array($widget_type, dependent_fields_allowed_widgets($field['type'], 'controlled'))) {
    return;
  }

  $type = (array)content_types($form['type_name']['#value']);

  dependent_fields_field_settings_form($form['field'], $field, $type);

  dependent_fields_widget_settings_form($form['widget'], $widget, $field, $type);

  return;
}

/**
 * Implementation of hook_widget_settings_alter().
 */

function _dependent_fields_widget_settings_alter(&$settings, $op, $widget) {
  // Only support modules that implement hook_dependent_fields_widgets().
  $widget_type = isset($widget['widget_type']) ? $widget['widget_type'] : $widget['type'];
  $allowed_widgets = dependent_fields_allowed_widgets('', 'controlled');

  if (!in_array($widget_type, $allowed_widgets)) {
    return;
  }

  // We don't use the form op here as it doesn't supply type and field information.
  // Instead we use the content_edit_form to create the dependent_fields_widget_
  // settings_form hook.

  if ($op == 'save') {
    $settings = array_merge($settings, dependent_fields_widget_settings($widget));
  }
}

/**
 * A list of settings needed by Dependent Fields module on widgets.
 */
function dependent_fields_widget_settings($widget) {
  $settings = array(
    'dependent_fields',
  );
  return $settings;
}

/**
 * Configuration form for editing the field settings for a Dependent Field.
 */
function dependent_fields_field_settings_form(&$form, $field, $type) {
  $params = array($field, $type);
  $field_settings = dependent_fields_invoke_field($field['type'], 'field_form', $params);
  if (!empty($field_settings)) {
    $form['dependent_fields'] = array(
      '#type' => 'fieldset',
      '#title' => t('Dependent Fields'),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
      '#weight' => 15,
    );
    $form['dependent_fields'] = array_merge($form['dependent_fields'], $field_settings);
  }
}

/**
 * Configuration form for editing the field settings for a Dependent Field.
 */
function dependent_fields_widget_settings_form(&$form, $widget, $field, $type) {
  $controller_field = isset($widget['dependent_fields']['controller_field']) ? $widget['dependent_fields']['controller_field'] : '';
  $hide_if_empty = isset($widget['dependent_fields']['hide_if_empty']) ? $widget['dependent_fields']['hide_if_empty'] : FALSE;
  $hide_none = isset($widget['dependent_fields']['hide_none']) ? $widget['dependent_fields']['hide_none'] : FALSE;

  $controlled_fields = dependent_fields_controlled_fields($field, $type);

  // Add the fieldset to the form
  $form['dependent_fields'] = array(
    '#type' => 'fieldset',
    '#title' => t('Dependent Fields'),
    '#collapsible' => TRUE,
    '#collapsed' => empty($controller_field) && empty($controlled_fields),
    '#weight' => 15,
    '#attributes' => array('id' => 'dependent-field-settings'),
    '#tree' => TRUE,
  );

  $fields = dependent_fields_list(FALSE);

  $potential_fields = dependent_fields_potential_controller_options($field, $type);

  if (!empty($potential_fields)) {
    $form['dependent_fields']['controller_field'] = array(
      '#type' => 'select',
      '#title' => t('Controller Field'),
      '#description' => t('Choose a field to act as a controller field.'),
      '#options' => $potential_fields,
      '#default_value' => $controller_field,
    );

    $form['dependent_fields']['hide_if_empty'] = array(
      '#type' => 'checkbox',
      '#title' => t('Hide if Empty'),
      '#description' => t('Should this field be hidden if the list of allowed options is empty.'),
      '#default_value' => $hide_if_empty,
    );
    $form['dependent_fields']['hide_none'] = array(
      '#type' => 'checkbox',
      '#title' => t('Hide None Option'),
      '#description' => t('For non-required fields, should the None option be hidden from select and button lists'),
      '#default_value' => $hide_none,
    );
    $params = array($widget, $field, $type);
    $form['dependent_fields'] = array_merge($form['dependent_fields'], dependent_fields_invoke_field($field['type'], 'widget_form', $params));
  }
  else {
    $form['dependent_fields']['controller_field'] = array(
      '#type' => 'value',
      '#default_value' => '',
    );
    $form['dependent_fields']['controller_field_empty'] = array(
      '#type' => 'markup',
      '#value' => t('No suitable controller fields in this content type.'),
      '#prefix' => '<p class="dependent-field-empty-text">',
      '#suffix' => '</p>',
    );
  }

  $controlled_fields = dependent_fields_controlled_fields_list($field, $type);

  if (!empty($controlled_fields)) {
    $form['dependent_fields']['controlled_fields'] = array(
      '#type' => 'markup',
      '#value' => t('This field acts as a controller field to the following fields:') . theme_item_list($controlled_fields),
      '#prefix' => '<div class="dependent-fields-controlled-fields">',
      '#suffix' => '</div>',
    );
  }
}