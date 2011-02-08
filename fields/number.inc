<?php
// $Id: number.inc,v 1.1.2.5 2011/01/09 18:13:32 smoothify Exp $

/**
 * @file
 * A Dependent Fields extension for the number field
 *
 * The "hooks" in this file are not true hooks, they're called individually
 * from the main dependent_fields.module in the corresponding hook by the
 * same name. Any of these hooks could be broken out into a separate module.
 */

/**
 * Implementation of hook_dependent_fields_info().
 */
function dependent_fields_number_info() {
  $field = array();
  $field['number_integer'] = array(
    'name' => t('Integer Field'),
    'controller_widgets' => array(
      'number',
      'optionwidgets_select',
      'optionwidgets_buttons',
      'optionwidgets_onoff'
    ),
    'controlled_widgets' => array(
      'number',
      'optionwidgets_select',
      'optionwidgets_buttons',
      'optionwidgets_onoff'
    ),
  );
  $field['number_float'] = array(
    'name' => t('Float Field'),
    'controller_widgets' => array(
      'number',
      'optionwidgets_select',
      'optionwidgets_buttons',
      'optionwidgets_onoff'
    ),
    'controlled_widgets' => array(
      'number',
      'optionwidgets_select',
      'optionwidgets_buttons',
      'optionwidgets_onoff'
    ),
  );
  $field['number_decimal'] = array(
    'name' => t('Decimal Field'),
    'controller_widgets' => array(
      'number',
      'optionwidgets_select',
      'optionwidgets_buttons',
      'optionwidgets_onoff'
    ),
    'controlled_widgets' => array(
      'number',
      'optionwidgets_select',
      'optionwidgets_buttons',
      'optionwidgets_onoff'
    ),
  );
  return $field;
}

/* Integer Field */

/**
 * Implementation of hook_dependent_fields_widget_form().
 */
function dependent_fields_number_integer_widget_form($widget, $field, $type) {
  return dependent_fields_number_widget_form($widget, $field, $type);
}


/**
 * Implementation of hook_dependent_fields_retrieve_field_value().
 */
function dependent_fields_number_integer_retrieve_field_value($field, $form_value, $form_state) {
  return dependent_fields_number_retrieve_field_value($field, $form_value, $form_state);
}


/**
 * Implementation of hook_dependent_fields_retrieve_field_keys().
 */
function dependent_fields_number_integer_retrieve_field_keys($field, $values) {
  return dependent_fields_number_retrieve_field_keys($field, $values);
}

/**
 * Implementation of hook_dependent_fields_retrieve_field_titles().
 */
function dependent_fields_number_integer_retrieve_field_titles($field, $values) {
  return dependent_fields_number_retrieve_field_titles($field, $values);
}

/**
 * Implementation of hook_allowed_values().
 */
function dependent_fields_number_integer_allowed_values($field, $info) {
  return dependent_fields_number_allowed_values($field, $info);
}


/* Float Field */

/**
 * Implementation of hook_dependent_fields_widget_form().
 */
function dependent_fields_number_float_widget_form($widget, $field, $type) {
  return dependent_fields_number_widget_form($widget, $field, $type);
}


/**
 * Implementation of hook_dependent_fields_retrieve_field_value().
 */
function dependent_fields_number_float_retrieve_field_value($field, $form_value, $form_state) {
  return dependent_fields_number_retrieve_field_value($field, $form_value, $form_state);
}


/**
 * Implementation of hook_dependent_fields_retrieve_field_keys().
 */
function dependent_fields_number_float_retrieve_field_keys($field, $values) {
  return dependent_fields_number_retrieve_field_keys($field, $values);
}

/**
 * Implementation of hook_dependent_fields_retrieve_field_titles().
 */
function dependent_fields_number_float_retrieve_field_titles($field, $values) {
  return dependent_fields_number_retrieve_field_titles($field, $values);
}

/**
 * Implementation of hook_allowed_values().
 */
function dependent_fields_number_float_allowed_values($field, $info) {
  return dependent_fields_number_allowed_values($field, $info);
}

/* Decimal Field */

/**
 * Implementation of hook_dependent_fields_widget_form().
 */
function dependent_fields_number_decimal_widget_form($widget, $field, $type) {
  return dependent_fields_number_widget_form($widget, $field, $type);
}


/**
 * Implementation of hook_dependent_fields_retrieve_field_value().
 */
function dependent_fields_number_decimal_retrieve_field_value($field, $form_value, $form_state) {
  return dependent_fields_number_retrieve_field_value($field, $form_value, $form_state);
}


/**
 * Implementation of hook_dependent_fields_retrieve_field_keys().
 */
function dependent_fields_number_decimal_retrieve_field_keys($field, $values) {
  return dependent_fields_number_retrieve_field_keys($field, $values);
}

/**
 * Implementation of hook_dependent_fields_retrieve_field_titles().
 */
function dependent_fields_number_decimal_retrieve_field_titles($field, $values) {
  return dependent_fields_number_retrieve_field_titles($field, $values);
}

/**
 * Implementation of hook_allowed_values().
 */
function dependent_fields_number_decimal_allowed_values($field, $info) {
  return dependent_fields_number_allowed_values($field, $info);
}


/* Generic Number Functions called above */

/**
 * Implementation of hook_dependent_fields_widget_form().
 */
function dependent_fields_number_widget_form($widget, $field, $type) {
  $form = array();
  $allowed_mapping = isset($widget['dependent_fields']['allowed_mapping']['mapping']) ? $widget['dependent_fields']['allowed_mapping']['mapping'] : '';

  $form['allowed_mapping'] = array(
    '#type' => 'fieldset',
    '#title' => t('Allowed values mapping'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );

  $form['allowed_mapping']['mapping'] = array(
    '#type' => 'textarea',
    '#title' => t('Controller field value mappping'),
    '#default_value' => $allowed_mapping,
    '#required' => FALSE,
    '#rows' => 8,
    '#description' => t('Enter the mappings between this field and the controller field. Enter one mapping per line, in the format controller_key|key1,key2,key3,key4. The controller_key is the value from the controller field, and key1, key2, key3, should be in this fields allowed values.'),
  );

  return $form;
}


/**
 * Implementation of hook_dependent_fields_retrieve_field_value().
 */
function dependent_fields_number_retrieve_field_value($field, $form_value, $form_state) {
  $return = array();

  if (empty($form_value)) {
    return $return;
  }

  $multiple = isset($form_value[0]);

  if ($multiple) {
    foreach ($form_value as $delta => $item) {
      if (!is_numeric($delta) || isset($item)) {
        continue;
      }
      $return[] = dependent_fields_number_retrieve_field_value($field, $item, $form_state);
    }
  }
  else {
    // Make sure we get the actual form element value.
    if (isset($form_value['value'])) {
      $return = array('value' => $form_value['value']);
    }
  }
  return $return;
}


/**
 * Implementation of hook_dependent_fields_retrieve_field_keys().
 */
function dependent_fields_number_retrieve_field_keys($field, $values) {
  $return = array();

  foreach ($values as $value) {
    // Locate each value.
    if (is_array($value) && isset($value['value'])) {
      $return[] = $value['value'];
    }
    elseif (isset($value)) {
      $return[] = $value;
    }
  }
  return $return;
}

/**
 * Implementation of hook_dependent_fields_retrieve_field_titles().
 */
function dependent_fields_number_retrieve_field_titles($field, $values) {
  $return = array();
  $all_values = content_allowed_values($field);
  foreach ($values as $value) {
    $key = '';
    if (is_array($value) && !empty($value['value'])) {
      $key = $value['value'];
    }
    elseif (!empty($value)) {
      $key = $value;
    }
    if (!empty($key) && !empty($all_values[$key])) {
      $return[] = $all_values[$key];
    }
  }
  return $return;

}

/**
 * Implementation of hook_allowed_values().
 */
function dependent_fields_number_allowed_values($field, $info) {
  $keys = dependent_fields_retrieve_field_keys($info['controller_field'], $info['controller_value']);
  $all_values = content_allowed_values($field);
  $mapping = dependent_fields_allowed_values_mapping($field);
  $allowed_values = dependent_fields_filter_allowed_values($all_values, $mapping, $keys);
  return $allowed_values;
}
