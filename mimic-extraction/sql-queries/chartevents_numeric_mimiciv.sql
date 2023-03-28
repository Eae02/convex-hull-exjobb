CREATE TABLE `project.dataset.chartevents_num` AS
SELECT subject_id, stay_id AS icustay_id, itemid, charttime, valuenum, valueuom FROM `project.dataset.chartevents` WHERE valuenum IS NOT NULL;