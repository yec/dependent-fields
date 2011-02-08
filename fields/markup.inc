<?php
// $Id: markup.inc,v 1.1.2.2 2011/01/09 18:13:32 smoothify Exp $

/**
 * @file
 * A Dependent Fields extension for the markup field
 *
 * The "hooks" in this file are not true hooks, they're called individually
 * from the main dependent_fields.module in the corresponding hook by the
 * same name. Any of these hooks could be broken out into a separate module.
 */

/**
 * Implementation of hook_dependent_fields_info().
 */
function dependent_fields_markup_info() {
  $field = array();
  $field['markup'] = array(
    'name' => t('Markup'),
    'controlled_widgets' => array(
      'markup',
    ),
  );
  return $field;
}


/**
 * Implementation of hook_dependent_fields_widget_form().
 */
function dependent_fields_markup_widget_form($widget, $field, $type) {
  $form = array();
  $markup = isset($widget['dependent_fields']['markup']) ? $widget['dependent_fields']['markup'] : '';
  $markup_php = isset($widget['dependent_fields']['advanced_options']['markup_php']) ? $widget['dependent_fields']['advanced_options']['markup_php'] : '';

  $form['markup'] = array(
    '#type' => 'textarea',
    '#title' => t('Markup Override'),
    '#default_value' => $markup,
    '#rows' => 15,
    '#description' => t('The markup to be displayed. Any HTML is legal here, so be careful not to break your page layout.'),
  );
  
  $form['markup_tokens_help'] = array(
    '#value' => dependent_fields_string_tokens_help(),
  );

  if (user_access('Use PHP input for field settings (dangerous - grant with care)')) {
    $form['advanced_options']['markup_php'] = array(
      '#type' => 'textarea',
      '#title' => t('Code'),
      '#default_value' => $markup_php,
      '#rows' => 8,
      '#description' => t('Advanced usage only: PHP code to replace HTML markup in the above field. Should include &lt;?php ?&gt; delimiters. You can access the current node object using $node.'),
    );
  }
  else {
    $form['advanced_options']['markup_php'] = array(
      '#type' => 'item',
      '#title' => t('Code'),
      '#value' => !empty($markup_php) ? '<code>'. check_plain($markup_php) .'</code>' : t('&lt;none&gt;'),
      '#description' => empty($field['markup_php']) ? t("You're not allowed to input PHP code.") : t('This PHP code was set by an administrator and will override the markup field above.'),
    );
  }
  
  return $form;
}

/* markup field specific functions */

/**
 * Implementation of hook_dependent_fields_build_field_info().
 */
function dependent_fields_markup_build_field_info($info, $field, $widget, $type) {
  $return = array();
  $markup = isset($widget['dependent_fields']['markup']) ? $widget['dependent_fields']['markup'] : '';
  $markup_php = isset($widget['dependent_fields']['advanced_options']['markup_php']) ? $widget['dependent_fields']['advanced_options']['markup_php'] : '';
  $return['markup'] = $markup;
  $return['markup_php'] = $markup_php;
  return $return;
}


/**
 * A preprocess callback for the markup form element
 */
function dependent_fields_markup_element_preprocess($element, $info, $form_state, $form) {
  if (empty($info['controller_field'])) {
    return $element;
  }
  if (!empty($info['markup']) || !empty($info['markup_php'])) {
    $element['#markup_callback'] = 'dependent_fields_markup_get_output';
  }

  return $element;
}

function dependent_fields_markup_get_output($element, $field, $form_state, $form) {
  $output = '';
  $info = dependent_fields_build_field_info($field, $form_state);
  if (!empty($info)) {
    // Check to see if there is a php value set, if so run it.
    if (!empty($info['markup_php'])) {
      // Create node object
      $node = array();
      // Check we are on a node edit form.
      if ($form['#id'] == 'node-form') {
        // Build a node element from the $form_state
        $node = markup_build_node($form, $form_state);
      }
      // Todo - review what we pass in here and perhaps make an easier to access
      // info object.
      $arguments = array(
        'node' => $node,
        'element' => $element,
        'field' => $field,
        'form_state' => $form_state,
        'info' => $info,
      );
      $output = markup_eval($info['markup_php'], $arguments);
    }
    elseif (!empty($info['markup'])) {
      $output = dependent_fields_replace_string_tokens($info, $info['markup']);
    }
  }
  return $output;
}